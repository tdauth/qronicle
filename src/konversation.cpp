#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QString>
#include <QStringView>
#include <QUtf8StringView>
#include <QStandardPaths>
#include <QRegularExpression>
#include <QtConcurrent>

#include "konversation.hpp"

namespace qronicle {

QString Konversation::id() const {
    return "konversation";
}

Messenger::Messages Konversation::loadFile(const QString &filePath) {
    Messages messages;

    QFile file(filePath);
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return messages;
    }

    QFileInfo fileInfo(filePath);
    QString channel = fileInfo.completeBaseName();

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    qint64 lineNumber = 1;
    QString absoluteFilePath = fileInfo.absoluteFilePath();

    while (!in.atEnd()) {
        QString line(in.readLine());

        if (line.startsWith("[")) {
            //qDebug() << "Konversation log line" << line;
            // [Samstag 2009-12-19] [12:12:39] Information	Es wird nach Server chat.freenode.net (Port 8001) gesucht ...
            // [Montag 2024-06-17] [14:30:10] <PartnerNick> Hallo, bist du da?
            // [Montag 2024-06-17] [14:30:45] <DeinNick> Ja, ich bin hier.
            static QRegularExpression re(R"(^\s*\[(?:.*?\s+)?(?<date>\d{4}-\d{2}-\d{2})\]\s*\[(?<time>[^\]]+)\]\s+<?(?<sender>[^>\t\s]+)>?\t(?<content>.*)$)");
        
            QRegularExpressionMatch match = re.match(line);
        
            if (match.hasMatch()) {
                //qDebug() << "Matched Konversation log line" << line;
                
                QStringView date = match.capturedView("date"); // 2009-12-19
                QStringView time = match.capturedView("time"); // 12:12:39
                QStringView sender = match.capturedView("sender"); // Information
                QStringView content = match.capturedView("content"); // Es wird nach Server chat.freenode.net (Port 8001) gesucht ...
                
                Message message;
                
                QString dateTimeStr = QString("%1T%2").arg(date, time);
                QDateTime dateTime = QDateTime::fromString(dateTimeStr, Qt::ISODate);
                dateTime.setTimeZone(QTimeZone::systemTimeZone()); // Your local system time of the logs.

                if (dateTime.isValid()) {
                    message.setTimestamp(dateTime);
                } else {
                    qWarning() << "Parsing failed for Konversation date time:" << dateTimeStr;
                    message.setTimestamp(fileInfo.lastModified());
                }
                
                message.setFilePath(absoluteFilePath);
                message.setLineNumber(lineNumber);
                message.setProtocol("Konversation");
                message.setMessenger("Konversation");
                message.setOut(false);
                message.setSource(sender.toString());
                message.setSourceNick(sender.toString());
                message.setDestination(QObject::tr("all")); // TODO Detect private messages
                message.setDestinationNick(QObject::tr("all"));
                message.setContent(content.toString());
                message.setContentHtml(content.toString());

                messages.push_back(std::move(message));
            } else {
                qWarning() << "Ignoring Konversation log line in file" << absoluteFilePath << ":" << lineNumber << line;
            }
        }

        lineNumber++;
    }

    return messages;
}

Messenger::Messages Konversation::loadDirectories(const QStringList &dirPaths) {
    QStringList filePaths;

    for (const QString &dir : dirPaths) {
        QDirIterator it(dir, QStringList() << "*.log", QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            filePaths << it.next();
        }
    }

    if (filePaths.isEmpty()) {
        qWarning() << "Did not find any Konversation .log files in the given directories!";
        return {};
    }

    qDebug() << "Loading" << filePaths.size() << "Konversation files concurrently...";

    return QtConcurrent::blockingMappedReduced<Messenger::Messages>(
        filePaths,
        [this](const QString &path) {
            return loadFile(path);
        },
        [](Messenger::Messages &result, const Messenger::Messages &intermediate) {
            result.append(intermediate);
        },
        QtConcurrent::UnorderedReduce
    );
}

QStringList Konversation::defaultDirectories() {
    return {
        QDir::homePath() + "/.local/share/konversation/logs",
        QDir::homePath() + "/.kde/share/apps/konversation/logs",
        QDir::homePath() + "/snap/konversation/common/.local/share/konversation/logs",
    };
}

}
