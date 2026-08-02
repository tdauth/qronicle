#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtConcurrent>
#include <QTimeZone>

#include "signal.hpp"

namespace qronicle {

QString Signal::id() const {
    return "signal";
}

Messenger::Messages Signal::loadFile(const QString &filePath) {
    Messages messages;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open Signal file:" << filePath;
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

        if (rootObj.contains("account")) {
            QJsonObject account = rootObj.value("account").toObject();

            if (account.contains("givenName")) {
                ownerName = account.value("givenName").toString();
            } else if (account.contains("username")) {
                ownerName = account.value("username").toString();
            }
        } else if (rootObj.contains("recipient")) {
             QJsonObject recipient = rootObj.value("recipient").toObject();
             QString id = recipient.value("id").toString();

             if (recipient.contains("contact")) {
                QJsonObject contact = recipient.value("contact").toObject();

                if (contact.contains("systemGivenName")) {
                    QString systemGivenName = contact.value("systemGivenName").toString();

                    if (contact.contains("systemFamilyName")) {
                        QString systemFamilyName = contact.value("systemFamilyName").toString();

                        nickNames.insert(id, systemGivenName + " " + systemFamilyName);
                    } else {
                        nickNames.insert(id, systemGivenName);
                    }
                } else if (contact.contains("profileGivenName")) {
                    QString profileGivenName = contact.value("profileGivenName").toString();

                    if (contact.contains("profileFamilyName")) {
                        QString profileFamilyName = contact.value("profileFamilyName").toString();

                        nickNames.insert(id, profileGivenName + " " + profileFamilyName);
                    } else {
                        nickNames.insert(id, profileGivenName);
                    }
                }
             } else if (recipient.contains("self")) {
                ownerAccountId = id;
                nickNames.insert(id, ownerName);
             }
        } else if (rootObj.contains("chat")) {
            QJsonObject chat = rootObj.value("chat").toObject();

            chatAccountIds.insert(chat.value("id").toString(), chat.value("recipientId").toString());
        } else if (rootObj.contains("chatItem")) {
            QJsonObject chatItem = rootObj.value("chatItem").toObject();

            // Parse only standard messages for now.
            // TODO parse "attachments": Create file URLs by comparing the locatorInfo.plaintextHash with the matching SHA256 hash of the file in the files folder of the directory of the main.jsonl file.
            if (chatItem.contains("standardMessage")) {
                QJsonObject standardMessage = chatItem.value("standardMessage").toObject();
                QString chatId = chatItem.value("chatId").toString();

                Message msg;
                msg.setFilePath(absoluteFilePath);
                msg.setLineNumber(lineNumber);
                msg.setProtocol("Signal");
                msg.setMessenger("Signal");
                msg.setOut(chatItem.contains("outgoing"));
                QTimeZone utcZone("UTC");
                msg.setTimestamp(QDateTime::fromMSecsSinceEpoch(chatItem.value("dateSent").toString().toLongLong(), utcZone));

                if (chatItem.contains("authorId")) {
                    QString authorId = chatItem.value("authorId").toString();

                    if (msg.out()) {
                        QString chatAccountId = chatAccountIds[chatId];
                        msg.setDestination(chatAccountId);

                        if (nickNames.contains(chatAccountId)) {
                            msg.setDestinationNick(nickNames[chatAccountId]);
                        } else {
                            msg.setDestinationNick(QObject::tr("Unknown"));
                        }

                        msg.setSource(ownerAccountId);
                        msg.setSourceNick(ownerName);
                    } else {
                        msg.setDestination(ownerAccountId);
                        msg.setDestinationNick(ownerName);
                        msg.setSource(authorId);

                        if (nickNames.contains(authorId)) {
                            msg.setSourceNick(nickNames[authorId]);
                        } else {
                            msg.setSourceNick(QObject::tr("Unknown"));
                        }
                    }
                }

                if (standardMessage.contains("text")) {
                    QJsonObject text = standardMessage.value("text").toObject();

                    if (text.contains("body")) {
                        QString content = text.value("body").toString();
                        msg.setContent(content);
                        msg.setContentHtml(formatHtml(content));
                    }
                }

                messages.append(msg);
            }
        }
    }

    file.close();

    return messages;
}

Messenger::Messages Signal::loadDirectories(const QStringList &dirPaths) {
    QStringList jsonFiles;

    for (const QString &dir : dirPaths) {
        QDirIterator jsonIt(dir, QStringList() << "*.jsonl", QDir::Files, QDirIterator::Subdirectories);
        while (jsonIt.hasNext()) {
            jsonFiles << jsonIt.next();
        }
    }

    if (jsonFiles.isEmpty()) {
        qDebug() << "Found no Signal JSONL files in given dirs!";
        return {};
    }

    qDebug() << "Parsing" << jsonFiles.size() << "Signal JSONL files concurrently...";

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
