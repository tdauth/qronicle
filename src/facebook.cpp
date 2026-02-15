#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtConcurrent>

#include "facebook.hpp"

namespace chronicle {
    
QString Facebook::id() const {
    return "facebook";
}

Messenger::Messages Facebook::loadFile(const QString &filePath) {
    Messages messages;
    
    // get info from file and dir names
    QFileInfo fileInfo(filePath);
   
    QString partner = QObject::tr("Unknown");
    QDir parentDir = fileInfo.dir(); 
    QString contact = parentDir.dirName();
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open Facebook Messenger file:" << filePath;
        return messages;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    file.close();

    if (doc.isNull()) {
        qWarning() << "JSON error in" << filePath << ":" << error.errorString();
        return messages;
    }
    
    QJsonObject rootObj = doc.object();
    
    QJsonArray participantsArray = rootObj.value("participants").toArray();

    QStringList participantNames;

    for (const QJsonValue &value : participantsArray) {
        QJsonObject participantObj = value.toObject();
        QString name = QString::fromUtf8(participantObj.value("name").toString().toLatin1());
        
        if (!name.isEmpty()) {
            participantNames.append(name);
        }
    }
    
    QString title = QString::fromUtf8(rootObj.value("title").toString(QObject::tr("Unknown")).toLatin1());

    QJsonArray messagesArray = rootObj.value("messages").toArray();
    
    for (const QJsonValue &value : messagesArray) {
        QJsonObject obj = value.toObject();
        
        Message msg;
        msg.setProtocol("Facebook Messenger");
        msg.setMessenger("Facebook Messenger");
        QString senderName = QString::fromUtf8(obj.value("sender_name").toString(QObject::tr("Unknown")).toLatin1());
        
        QString otherParticipant;

        for (const QString &name : participantNames) {
            if (name != senderName) {
                otherParticipant = name;
                break;
            }
        }
        
        if (senderName == title) {
            // in
            msg.setSource(contact);
            msg.setSourceNick(senderName);
            msg.setDestination(otherParticipant);
            msg.setDestinationNick(otherParticipant);
        } else {
            // out
            msg.setSource(senderName);
            msg.setSourceNick(senderName);
            msg.setDestination(contact);
            msg.setDestinationNick(otherParticipant);
        }
        
        msg.setTimestamp(QDateTime::fromMSecsSinceEpoch(obj.value("timestamp_ms").toVariant().toLongLong()));
        QString content = QString::fromUtf8(obj.value("content").toString().toLatin1());
        msg.setContent(content);
        
        QString contentHtml = content;
        
        // URLs into HTML
        if (contentHtml.contains("http://") || contentHtml.contains("https://")) {
            static QRegularExpression urlRegex(R"((https?:\/\/[^\s\n\r]+))");
            contentHtml.replace(urlRegex, R"(<a href="\1">\1</a>)");
        }
        
        msg.setContentHtml(contentHtml);
        
        messages.append(msg);
    }

    return messages;
}

Messenger::Messages Facebook::loadDirectories(const QStringList &dirPaths) {
    QStringList jsonFiles;

    // 1. Alle relevanten JSON-Dateien sammeln
    for (const QString &dir : dirPaths) {
        QDirIterator it(dir, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        
        while (it.hasNext()) {
            QString currentDirPath = it.next();
            // Nur Ordner namens "messages" scannen
            if (QFileInfo(currentDirPath).fileName() == "messages") {
                QDirIterator jsonIt(currentDirPath, QStringList() << "*.json", QDir::Files, QDirIterator::Subdirectories);
                while (jsonIt.hasNext()) {
                    jsonFiles << jsonIt.next();
                }
            }
        }
    }

    if (jsonFiles.isEmpty()) return {};

    qDebug() << "Parsing" << jsonFiles.size() << "Facebook JSON files in parallel...";

    // 2. Parallelisiertes Laden und Zusammenführen
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

QStringList Facebook::defaultDirectories() {
    return {};
}

}
