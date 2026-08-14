#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QCryptographicHash>
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

namespace {

QByteArray sha256FileBase64(const QString& filePath) {
    QFile f(filePath);

    if (!f.open(QIODevice::ReadOnly)) {
        return {};
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);

    while (!f.atEnd()) {
        hash.addData(f.read(1024 * 1024));
    }

    return hash.result().toBase64();
}

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

    // The sub directory "files" contains all media.
    QHash<QByteArray, QString> media;
    QString filesDir = QDir(absoluteDirPath).absoluteFilePath("files");
    QDirIterator dirIt(filesDir, QStringList() << "*", QDir::Files, QDirIterator::Subdirectories);
    while (dirIt.hasNext()) {
        QString mediaFilePath = dirIt.next();
        QByteArray sha256Base64 = sha256FileBase64(mediaFilePath);

        if (!sha256Base64.isEmpty()) {
            media.insert(sha256Base64, mediaFilePath);
        }
    }

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

                QString content;

                if (standardMessage.contains("text")) {
                    QJsonObject text = standardMessage.value("text").toObject();

                    if (text.contains("body")) {
                        content = text.value("body").toString();
                    }
                }

                if (standardMessage.contains("attachments")) {
                    for (const QJsonValue &v : standardMessage.value("attachments").toArray()) {
                        if (v.isObject()) {
                            QJsonObject attachment = v.toObject();

                            if (attachment.contains("pointer")) {
                                QJsonObject pointer = attachment.value("pointer").toObject();

                                if (pointer.contains("locatorInfo")) {
                                    QJsonObject locatorInfo = pointer.value("locatorInfo").toObject();

                                    if (locatorInfo.contains("plaintextHash")) {
                                        QByteArray plaintextHash = locatorInfo.value("plaintextHash").toString().trimmed().toLatin1();
                                        auto it = media.find(plaintextHash);

                                        if (it != media.end()) {
                                            content += QUrl::fromLocalFile(it.value()).toString();
                                        } else {
                                            content += QUrl::fromLocalFile(QDir(filesDir).absoluteFilePath(plaintextHash)).toString();

                                            qWarning() << "Missing Signal media" << plaintextHash;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                msg.setContent(content);
                msg.setContentHtml(formatHtml(content));

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
