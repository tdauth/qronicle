#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QXmlStreamReader>
#include <QString>
#include <QStringView>
#include <QUtf8StringView>
#include <QStandardPaths>
#include <QRegularExpression>
#include <QtConcurrent>

#include "trillian.hpp"

namespace qronicle {

namespace {

QDateTime getTimestamp(const QString &time) {
    bool ok;
    qint64 msecs = time.toLongLong(&ok);
    if (!ok) {
        qWarning() << "Invalid Trillian timestamp:" << time;
        return QDateTime();
    }
    // Falls Kopete Sekunden statt Millisekunden nutzt (1185831353 = 2007)
    // Wenn die Zahl klein ist, als Sekunden interpretieren:
    if (msecs < 10000000000LL) {
        return QDateTime::fromSecsSinceEpoch(msecs);
    }

    return QDateTime::fromMSecsSinceEpoch(msecs);
}

QString decodeMessage(const QString &message) {
    if (!message.contains('%')) {
        return message; // Schneller Ausstieg, falls nichts zu decodieren ist
    }
    // fromPercentEncoding wandelt %3C in <, %22 in " usw. um
    return QUrl::fromPercentEncoding(message.toUtf8());
}

}

QString Trillian::id() const {
    return "trillian";
}

Messenger::Messages Trillian::loadFile(const QString &filePath) {
    Messages messages;

    QFileInfo fileInfo(filePath);

    if (!fileInfo.exists()) {
        return messages;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return messages;
    }

    QByteArray data = file.readAll();

    QXmlStreamReader reader;
    // fake root element to parse
    reader.addData("<root>");
    reader.addData(data);
    reader.addData("</root>");

    QMap<QString, QString> participantNicknames;

    QString owner;
    QString other;

    while (!reader.atEnd()) {
        reader.readNext();

        if (reader.isStartElement()) {
            QStringView tagName = reader.name();

            if (tagName == QStringLiteral("session")) {
                auto attrs = reader.attributes();

                if (attrs.value(QStringLiteral("type")).toString() == "start") {
                    owner = attrs.value(QStringLiteral("from")).toString();
                    other = attrs.value(QStringLiteral("to")).toString();
                }
                
                Message message;
                message.setProtocol(attrs.value(QStringLiteral("medium")).toString());
                message.setMessenger("Trillian");
                message.setFilePath(fileInfo.absoluteFilePath());
                message.setLineNumber(reader.lineNumber());
                message.setSource(attrs.value(QStringLiteral("from")).toString());
                message.setSourceNick(message.source());
                message.setDestination(attrs.value(QStringLiteral("to")).toString());
                message.setDestinationNick(message.destination());
                message.setTimestamp(getTimestamp(attrs.value(QStringLiteral("time")).toString()));
                QString type = decodeMessage(attrs.value(QStringLiteral("type")).toString());
                message.setType(type);
                QString content = QObject::tr("Session changed to %1.").arg(type);
                message.setContent(content);
                message.setContentHtml(content);
                messages.push_back(std::move(message));
            } else if (tagName == QStringLiteral("status")) {
                auto attrs = reader.attributes();
                Message message;
                message.setProtocol(attrs.value(QStringLiteral("medium")).toString());
                message.setMessenger("Trillian");
                message.setFilePath(fileInfo.absoluteFilePath());
                message.setLineNumber(reader.lineNumber());
                message.setSource(attrs.value(QStringLiteral("from")).toString());
                message.setSourceNick(message.source());
                message.setDestination(attrs.value(QStringLiteral("from")).toString());
                message.setDestinationNick(message.destination());
                message.setTimestamp(getTimestamp(attrs.value(QStringLiteral("time")).toString()));
                QString type = decodeMessage(attrs.value(QStringLiteral("type")).toString());
                message.setType(type);
                QString content = QObject::tr("Status changed to %1.").arg(type);
                message.setContent(content);
                message.setContentHtml(content);
                messages.push_back(std::move(message));
            } else if (tagName == QStringLiteral("icon")) {
                auto attrs = reader.attributes();
                Message message;
                message.setProtocol(attrs.value(QStringLiteral("medium")).toString());
                message.setMessenger("Trillian");
                message.setFilePath(fileInfo.absoluteFilePath());
                message.setLineNumber(reader.lineNumber());
                message.setSource(attrs.value(QStringLiteral("from")).toString());
                message.setSourceNick(message.source());
                message.setDestination(attrs.value(QStringLiteral("from")).toString());
                message.setDestinationNick(message.destination());
                message.setTimestamp(getTimestamp(attrs.value(QStringLiteral("time")).toString()));
                QString link = decodeMessage(attrs.value(QStringLiteral("link")).toString());
                message.setContent(link);
                message.setContentHtml(link);
                messages.push_back(std::move(message));
            } else if (tagName == QStringLiteral("message")) {
                Message message;
                auto attrs = reader.attributes();

                message.setProtocol(attrs.value(QStringLiteral("medium")).toString());
                message.setMessenger("Trillian");
                message.setFilePath(fileInfo.absoluteFilePath());
                message.setLineNumber(reader.lineNumber());

                QString type = attrs.value(QStringLiteral("type")).toString();
                message.setType(type);
                bool out = type == "outgoing_privateMessage";
                message.setOut(out);

                if (type == "information_standard") {
                    message.setSource(other);
                    message.setSourceNick(other);
                    message.setDestination(owner);
                    message.setDestinationNick(other);
                } else {
                    QString rawValue = attrs.value(QStringLiteral("from_display")).toString();
                    QString nick = decodeMessage(rawValue);
                    message.setSourceNick(nick);

                    message.setSource(attrs.value(QStringLiteral("from")).toString());
                    message.setDestination(attrs.value(QStringLiteral("to")).toString());

                    // Nickname-Cache
                    if (message.source() != message.sourceNick() && !participantNicknames.contains(message.source())) {
                        participantNicknames.insert(message.source(), nick);
                    }
                    message.setDestinationNick(participantNicknames.value(message.destination()));
                }

                message.setTimestamp(getTimestamp(attrs.value(QStringLiteral("time")).toString()));

                QString content = decodeMessage(attrs.value(QStringLiteral("text")).toString());
                message.setContent(content);

                message.setContentHtml(content); // Do not format HTML. It seems that Trillian already has HTML links.

                messages.push_back(std::move(message));
            } else {
                qWarning() << "Ignoring Trillian tag" << tagName << "in file" << fileInfo.absoluteFilePath() << reader.lineNumber();
            }
        }
    }
    
    //qDebug() << "Trillian participantNicknames:" << participantNicknames;
    
    // Sometimes the from_display will appear only later in the chat, so we refill here:
    for (auto &msg : messages) {
        if (participantNicknames.contains(msg.sourceNick())) {
            //qDebug() << "Trillian updating source" << msg.sourceNick() << "with" << participantNicknames.value(msg.sourceNick());
            msg.setSourceNick(participantNicknames.value(msg.sourceNick()));
        }
        
        if (participantNicknames.contains(msg.destinationNick())) {
            //qDebug() << "Trillian updating destination" << msg.destinationNick() << "with" << participantNicknames.value(msg.destinationNick());
            msg.setDestinationNick(participantNicknames.value(msg.destinationNick()));
        }
    }

    return messages;
}

Messenger::Messages Trillian::loadDirectories(const QStringList &dirPaths) {
    QStringList filePaths;

    // 1. Alle Verzeichnisse nach XML-Dateien durchsuchen
    for (const QString &dir : dirPaths) {
        QDirIterator it(dir, QStringList() << "*.xml", QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            filePaths << it.next();
        }
    }

    if (filePaths.isEmpty()) {
        qWarning() << "No Trillian XML files found in the given directories!";
        return {};
    }

    qDebug() << "Loading" << filePaths.size() << "Trillian files concurrently...";

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

QStringList Trillian::defaultDirectories() {
    return {
        QDir::homePath() + "/.config/trillian"
    };
}

}
