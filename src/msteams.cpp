#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QRegularExpression>
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

namespace {

/*
<URIObject type=\"Picture.1\" uri=\"https://api.asm.skype.com/v1/objects/filename\" url_thumbnail=\"https://api.asm.skype.com/v1/objects/filename/views/imgt1\">To view this shared photo, go to: <a href=\"https://login.skype.com/login/sso?go=webclient.xmm&amp;pic=filename\">https://login.skype.com/login/sso?go=webclient.xmm&amp;pic=filename</a><OriginalName v=\"test.jpg\"/><meta type=\"photo\" originalName=\"test.jpg\"/></URIObject>
*/
QString parseSkypePhotoToCustomLink(const QString &input, const QString &path, const QHash<QString, QString> &media) {
    static const QRegularExpression idRegex("uri=\"https://api\\.asm\\.skype\\.com/v1/objects/([^\"]+)\"");
    static const QRegularExpression nameRegex("originalName=\"([^\"]+)\"");

    QRegularExpressionMatch idMatch = idRegex.match(input);
    if (!idMatch.hasMatch()) {
        return input;
    }

    QRegularExpressionMatch nameMatch = nameRegex.match(input);

    QStringView objectId = idMatch.capturedView(1);
    QStringView fileName = nameMatch.hasMatch() ? nameMatch.capturedView(1) : QStringView(u"Shared Photo");
    QStringView nameStr = fileName;

    QString baseName = QDir(path).absoluteFilePath(objectId.toString());
    QString filePath = baseName;

    auto it = media.find(baseName);

    if (it != media.end()) {
        filePath = it.value();
    } else {
        //qWarning() << "Missing media for base name" << baseName;
    }

    return QString("<a href=\"file://%1\">%2</a>")
        .arg(filePath)
        .arg(nameStr);
}

}

Messenger::Messages MsTeams::loadFile(const QString &filePath) {
    Messages messages;

    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open MS Teams file:" << filePath;
        return messages;
    }

    QFileInfo fileInfo(filePath);
    QString absoluteFilePath = fileInfo.absoluteFilePath();
    QString absoluteDirPath = fileInfo.absolutePath();

    QHash<QString, QString> media;
    QString mediaDir = QDir(absoluteDirPath).absoluteFilePath("media");
    QDirIterator dirIt(mediaDir, QStringList() << "*", QDir::Files, QDirIterator::Subdirectories);
    while (dirIt.hasNext()) {
        QString mediaFilePath = dirIt.next();
        QString mediaBaseName = QFileInfo(mediaFilePath).baseName();
        QString absolutePathWithoutExtension = QDir(mediaDir).absoluteFilePath(mediaBaseName);
        media.insert(absolutePathWithoutExtension, mediaFilePath);
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);

    if (doc.isNull()) {
        qWarning() << "JSON error in" << filePath << ":" << error.errorString();
        return messages;
    }

    QJsonObject rootObj = doc.object();
    QString userId;

    if (rootObj.contains("userId")) {
        userId = rootObj.value("userId").toString();
    }

    QHash<QString, QString> nickNames;

    if (rootObj.contains("conversations") && rootObj.value("conversations").isArray()) {
        for (const QJsonValue &v : rootObj.value("conversations").toArray()) {
            if (v.isObject()) {
                QJsonObject conversation = v.toObject();

                if (conversation.contains("MessageList")  && conversation.value("MessageList").isArray()) {
                    for (const QJsonValue &v2 : conversation.value("MessageList").toArray()) {
                        if (v2.isObject()) {
                            QJsonObject message = v2.toObject();

                            if (message.contains("messagetype")) {
                                QString messageType = message.value("messagetype").toString();

                                // ignore other chat events for now
                                if (messageType.startsWith("RichText") || messageType.startsWith("Text")) { // RichText/Html
                                    Message msg;
                                    msg.setFilePath(absoluteFilePath);
                                    msg.setProtocol("MS Teams");
                                    msg.setMessenger("MS Teams");
                                    msg.setOut(message.contains("outgoing"));
                                    msg.setTimestamp(QDateTime::fromString(message.value("originalarrivaltime").toString(), Qt::ISODateWithMs));

                                    QString conversationId;

                                    if (message.contains("conversationid")) {
                                        conversationId = message.value("conversationid").toString();
                                    }

                                    QString from;

                                    if (message.contains("from")) {
                                        from = message.value("from").toString();
                                    }

                                    QString rawValue;

                                    if (message.contains("properties") && message.value("properties").isObject()) {
                                        QJsonObject properties = message.value("properties").toObject();

                                        if (properties.contains("importedBy") && properties.value("importedBy").isObject()) {
                                            QJsonObject importedBy = properties.value("importedBy").toObject();

                                            if (importedBy.contains("RawValue")) {
                                                rawValue = importedBy.value("RawValue").toString();
                                            }

                                            if (importedBy.contains("Network")) {
                                                msg.setProtocol(importedBy.value("Network").toString());
                                            }
                                        }
                                    }

                                    if (from.isEmpty() && !rawValue.isEmpty()) {
                                        from = rawValue;
                                    }

                                    msg.setSource(from);

                                    if (from == userId) {
                                        msg.setOut(true);
                                        msg.setSourceNick(userId);

                                        if (rawValue != userId) {
                                            msg.setDestination(rawValue);
                                        } else {
                                             if (nickNames.contains(conversationId)) {
                                                msg.setDestination(nickNames[conversationId]);
                                             }
                                        }

                                        if (message.contains("displayName") && !message.value("displayName").toString().isEmpty()) {
                                            msg.setDestinationNick(message.value("displayName").toString());
                                        } else {
                                            if (rawValue != userId) {
                                                msg.setDestinationNick(rawValue);
                                            } else {
                                                msg.setDestinationNick(msg.destination());
                                            }
                                        }
                                    } else {
                                        msg.setOut(false);
                                        msg.setDestination(userId);
                                        msg.setDestinationNick(userId);

                                        if (message.contains("displayName") && !message.value("displayName").toString().isEmpty()) {
                                            msg.setSourceNick(message.value("displayName").toString());
                                        } else {
                                            msg.setSourceNick(rawValue);
                                        }
                                    }

                                    if (message.contains("content")) {
                                        QString content = message.value("content").toString();
                                        msg.setContent(content);
                                        msg.setContentHtml(parseSkypePhotoToCustomLink(content, mediaDir, media));
                                    }

                                    messages.append(msg);

                                    if (!conversationId.isEmpty()) {
                                        if (msg.out()) {
                                            nickNames.insert(conversationId, msg.destinationNick());
                                        } else {
                                            nickNames.insert(conversationId, msg.sourceNick());
                                        }
                                    }
                                }
                            }
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
