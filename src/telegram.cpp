#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtConcurrent>

#include "telegram.hpp"

namespace qronicle {

QString Telegram::id() const {
    return "telegram";
}

Messenger::Messages Telegram::loadFile(const QString &filePath) {
    Messages messages;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open Telegram file:" << filePath;
        return messages;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    file.close();

    if (doc.isNull()) {
        qWarning() << "JSON error in" << filePath << ":" << error.errorString();
        return messages;
    }

    QFileInfo fileInfo(filePath);
    QString absoluteFilePath = fileInfo.absoluteFilePath();
    QString absoluteDirPath = fileInfo.absolutePath();

    QJsonObject rootObj = doc.object();
    QHash<QString, QString> nickNames;

    QJsonArray messagesArray = rootObj.value("messages").toArray();

    for (const QJsonValue &value : messagesArray) {
        QJsonObject obj = value.toObject();

        Message msg;
        msg.setFilePath(absoluteFilePath);
        msg.setProtocol("Telegram");
        msg.setMessenger("Telegram");
        msg.setTimestamp(QDateTime::fromSecsSinceEpoch(obj.value("date_unixtime").toVariant().toString().toLongLong()));
        QString sender = obj.contains("actor_id") ? obj.value("actor_id").toString(QObject::tr("Unknown")) : obj.value("from_id").toString(QObject::tr("Unknown"));
        QString senderName = obj.contains("actor") ? obj.value("actor").toString(QObject::tr("Unknown")) :obj.value("from").toString(QObject::tr("Unknown"));

        msg.setSource(sender);
        msg.setSourceNick(senderName);

        nickNames.insert(sender, senderName);

        QString content = obj.value("text").toString();
        QJsonArray text_entities = obj.value("text_entities").toArray();
        QString contentHtml;

        for (const QJsonValue &value : text_entities) {
            QJsonObject o = value.toObject();

             if (!o.contains("type") || !o.contains("text")) {
                qWarning() << "Missing key in text_entities:" << o;
                continue;
            }

            QString type = o.value("type").toString();
            QString text = o.value("text").toString();

            if (type == "plain") {
                contentHtml += text;
            } else if (type == "mention") {
                contentHtml += text;
            } else if (type == "bold") {
                contentHtml += "<b>" + text + "</b>";
            } else if (type == "link") {
                contentHtml += "<a href=\"" + text + "\">" + text + "</a>";
            } else if (type == "text_link") {
                QString href = o.value("href").toString();
                contentHtml += "<a href=\"" + href + "\">" + text + "</a>";
            } else {
                qWarning() << "Ignoring unsupported Telegram text type:" << type;
            }
        }

        if (obj.contains("photo")) {
            QString photo = obj.value("photo").toString();
            content += photo;
            contentHtml += "<a href=\"file://" + absoluteDirPath + "/" + photo + "\">" + photo + "</a>";
        }

        msg.setContent(content);
        msg.setContentHtml(contentHtml);
        messages.append(msg);
    }

    // Fill destinations:
    for (auto &msg : messages) {
        if (msg.destination().isEmpty()) {
            bool matches = false;

            for (auto i = nickNames.constBegin(); i != nickNames.constEnd(); ++i) {
                if (msg.source() != i.key()) {
                    msg.setDestination(i.key());
                    msg.setDestinationNick(i.value());

                    matches = true;

                    break;
                }
            }

            if (!matches) {
                msg.setDestination(QObject::tr("Unknown"));
                msg.setDestinationNick(QObject::tr("Unknown"));
            }
        }
    }

    return messages;
}

Messenger::Messages Telegram::loadDirectories(const QStringList &dirPaths) {
    QStringList jsonFiles;

    for (const QString &dir : dirPaths) {
        QDirIterator jsonIt(dir, QStringList() << "*.json", QDir::Files, QDirIterator::Subdirectories);
        while (jsonIt.hasNext()) {
            jsonFiles << jsonIt.next();
        }
    }

    if (jsonFiles.isEmpty()) {
        qDebug() << "Found no Telegram JSON files in given dirs!";
        return {};
    }

    qDebug() << "Parsing" << jsonFiles.size() << "Telegram JSON files concurrently...";

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
