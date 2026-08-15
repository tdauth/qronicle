#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtConcurrent>
#include <QTimeZone>

#include "msteams.hpp"

namespace qronicle {

QString MsTeams::id() const {
    return "msteams";
}

Messenger::Messages MsTeams::loadFile(const QString &filePath) {
    Messages messages;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open MS Teams file:" << filePath;
        return messages;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);

    QFileInfo fileInfo(filePath);
    QString absoluteFilePath = fileInfo.absoluteFilePath();
    QString absoluteDirPath = fileInfo.absolutePath();

    QHash<QString, QString> nickNames;
    QString ownerName;
    QString ownerAccountId;

    QHash<QString, QString> chatAccountIds;

    qint64 lineNumber = 0;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        lineNumber++;

        if (line.isEmpty()) {
            continue;
        }

        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8(), &error);

        if (doc.isNull()) {
            qWarning() << "JSON error in" << filePath << ":" << error.errorString();
            return messages;
        }

        QJsonObject rootObj = doc.object();

        if (rootObj.contains("conversations")) {
            QJsonObject conversations = rootObj.value("conversations").toObject();

            if (conversations.contains("MessageList")) {
                for (const QJsonValue &v : conversations.value("MessageList").toArray()) {
                    if (v.isObject()) {
                        QJsonObject message = v.toObject();

                        // ignore other chat events
                        if (message.contains("from")) {
                            Message msg;
                            msg.setFilePath(absoluteFilePath);
                            msg.setLineNumber(lineNumber);
                            msg.setProtocol("Microsoft Teams");
                            msg.setMessenger("Microsoft Teams");
                            msg.setOut(message.contains("outgoing"));
                            msg.setTimestamp(QDateTime::fromString(message.value("originalarrivaltime").toString(), Qt::ISODateWithMs));

                            if (message.contains("from")) {
                                msg.setSource(message.value("from").toString());

                                if (message.contains("displayName")) {
                                    msg.setSourceNick(message.value("displayName").toString());
                                }
                            }

                            if (message.contains("content")) {
                                QString content = message.value("content").toString();
                                msg.setContent(content);
                                msg.setContentHtml(formatHtml(content));
                            }

                            messages.append(msg);
                        }
                    }
                }
            }
        }
    }

    file.close();

    return messages;
}

Messenger::Messages MsTeams::loadDirectories(const QStringList &dirPaths) {
    QStringList jsonFiles;

    for (const QString &dir : dirPaths) {
        QDirIterator jsonIt(dir, QStringList() << "messages.json", QDir::Files, QDirIterator::Subdirectories);
        while (jsonIt.hasNext()) {
            jsonFiles << jsonIt.next();
        }
    }

    if (jsonFiles.isEmpty()) {
        qDebug() << "Found no MS Teams JSON files in given dirs!";
        return {};
    }

    qDebug() << "Parsing" << jsonFiles.size() << "MS Teams JSON files concurrently...";

    return QtConcurrent::blockingMappedReduced<Messenger::Messages>(
        jsonFiles,
        [this](const QString &filePath) {
            return loadFile(filePath);
        },
        [](Messenger::Messages &result, const Messenger::Messages &intermediate) {
            result.append(intermediate);
        },
        QtConcurrent::UnorderedReduce
    );
}

}
