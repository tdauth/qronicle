#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QtSql>
#include <QDateTime>
#include <QtConcurrent>

#include "knuddels.hpp"

namespace qronicle {

QString Knuddels::id() const {
    return "knuddels";
}

Messenger::Messages Knuddels::loadFile(const QString &filePath) {
    Messages messages;
    QFileInfo fileInfo(filePath);
    
    if (!fileInfo.exists()) {
        return messages;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return messages;
    }

    messages.reserve(file.size() / 100);
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);

    QSet<QString> participants;
    static QRegularExpression re(R"(^\((?<name>.+?) - (?<timestamp>[^)]+)\): (?<msg>.*)$)");
    qint64 lineNumber = 1;

    while (!in.atEnd()) {
        QString line = in.readLine();
        if (!line.isEmpty()) {
            QRegularExpressionMatch match = re.match(line);

            if (match.hasMatch()) {
                Message msg;
                msg.setProtocol("Knuddels");
                msg.setMessenger("Knuddels");
                msg.setFilePath(fileInfo.absoluteFilePath());
                msg.setLineNumber(lineNumber);
                QString name = match.captured("name");
                msg.setSource(name);
                msg.setSourceNick(name);
                msg.setTimestamp(QDateTime::fromString(match.captured("timestamp"), "dd.MM.yyyy HH:mm:ss"));
                QString message = match.captured("msg");
                msg.setContent(message);
                msg.setContentHtml(message);
                messages.append(std::move(msg));
                
                if (!participants.contains(name)) {
                    participants.insert(name);
                }
            }
        }
        
        lineNumber++;
    }

    // Destination-Logik optimieren
    if (participants.size() == 2) {
        QString p1 = *participants.begin();
        QString p2 = *(++participants.begin());

        for (Message &msg : messages) {
            const QString& other = (msg.source() == p1) ? p2 : p1;
            msg.setDestination(other);
            msg.setDestinationNick(other);
        }
    } else {
        // Fallback für Gruppen oder unbekannte Teilnehmer
        for (Message &msg : messages) {
            if (msg.destination().isEmpty()) {
                msg.setDestination("group/unknown");
            }
        }
    }

    return messages;
}

Messenger::Messages Knuddels::loadDirectories(const QStringList &dirPaths) {
    QStringList allFilePaths;

    for (const QString &dir : dirPaths) {
        QDirIterator it(dir, QStringList() << "*.txt", QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            allFilePaths << it.next();
        }
    }

    if (allFilePaths.isEmpty()) {
        return {};
    }

    qDebug() << "Loading" << allFilePaths.size() << "Knuddels files in parallel from" << dirPaths.size() << "directories...";

    return QtConcurrent::blockingMappedReduced<Messenger::Messages>(
        allFilePaths,
        [this](const QString &path) { 
            return loadFile(path); 
        },
        [](Messenger::Messages &result, const Messenger::Messages &intermediate) {
            result.append(intermediate);
        },
        QtConcurrent::UnorderedReduce
    );
}

QStringList Knuddels::defaultDirectories() {
    return {};
}

}
