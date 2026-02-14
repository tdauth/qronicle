#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QDomDocument>
#include <QStringView>
#include <QStandardPaths>

#include "kopete.hpp"

namespace chronicle {
    
QDateTime getTimestamp(int year, int month, QString time) {
    QStringList parts = time.split(' ');
    if (parts.size() < 2) {
        qDebug() << "Invalid date" << time;
        return QDateTime(); 
    }

    int day = parts.at(0).toInt();
    QString timeStr = parts.at(1); // "11:53:42"
    QDate date(year, month, day);
    
    return QDateTime(date, QTime::fromString(timeStr, "HH:mm:ss"));
}
    
Messenger::Messages Kopete::loadFile(const QString &filePath) {
    Messenger::Messages messages;
    
    // get info from file and dir names
    QFileInfo fileInfo(filePath);
    QStringView sv{fileInfo.completeBaseName()};
    int dotPos = sv.indexOf('.');
    int year = 0;
    int month = 0;
    QString partner = QObject::tr("Unknown");

    if (dotPos != -1) {
        partner = sv.left(dotPos).toString();
        QStringView datePart = sv.mid(dotPos + 1);
        if (datePart.length() >= 6) {
            year = datePart.left(4).toInt();
            month = datePart.mid(4, 2).toInt();
        }
    }
    
    QDir parentDir = fileInfo.dir(); 
    QString owner = parentDir.dirName();
    QString protocol = QObject::tr("Unknown");
    if (parentDir.cdUp()) {
        protocol = parentDir.dirName();
    }

    // Parse XML
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open" << filePath;
        return messages;
    }
    
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    QDomDocument doc;
    if (!doc.setContent(stream.readAll())) {
        qWarning() << "Cannot read DOM" << filePath;
        return messages;
    }
    
    file.close();

    QDomElement root = doc.documentElement();
    
    if (root.tagName() != "kopete-history") {
        qWarning() << "Unexpected root element:" << root.tagName();
    }
    
    QDomElement head = root.firstChildElement("head");

    if (!head.isNull()) {
        QDomElement dateElement = head.firstChildElement("date");
        
        if (!dateElement.isNull()) {
            QString monthStr = dateElement.attribute("month");
            QString yearStr = dateElement.attribute("year");

            month = monthStr.toInt();
            year = yearStr.toInt();
        }
    }
    
    QDomNodeList msg = root.childNodes();
    
    messages.reserve(msg.size());
    QMap<QString, QString> participantNicknames;

    for (int i = 0; i < msg.size(); ++i) {
        QDomElement msgElem = msg.at(i).toElement();
        
        if (msgElem.tagName() == "msg") {
            Message message;
            message.setProtocol(protocol);
            message.setSourceNick(msgElem.attribute("nick"));
            
            if (msgElem.attribute("in") == "1") {
                message.setSource(partner);
                message.setDestination(owner);
            } else {
                message.setSource(owner);
                message.setDestination(partner);
            }
            
            if (!participantNicknames.contains(message.source())) {
                participantNicknames.insert(message.source(), message.sourceNick());
            }
            
            message.setDestinationNick(participantNicknames.value(message.destination()));
            message.setTimestamp(getTimestamp(year, month, msgElem.attribute("time")));
            message.setContent(msgElem.text());
            messages.push_back(message);
        }
    }

    return messages;
}

Messenger::Messages Kopete::loadDirectory(const QString &dir) {
    Messenger::Messages allMessages;
    QDirIterator it(dir, QStringList() << "*.xml", QDir::Files, QDirIterator::Subdirectories);

    while (it.hasNext()) {
        QString filePath = it.next();
        qDebug() << "Loading Kopete file" << filePath;
        Messenger::Messages fileMessages = loadFile(filePath);
        
        if (!fileMessages.empty()) {
            allMessages.reserve(allMessages.size() + fileMessages.size());
            for(auto &&msg : fileMessages) {
                allMessages.append(std::move(msg));
            }
        }
    }

    return allMessages;
}

QStringList Kopete::defaultDirectories() {
    return {
        QDir::homePath() + "/.local/share/kopete/logs",
        QDir::homePath() + "/.kde/share/apps/kopete/logs",
        QDir::homePath() + "/.kde4/share/apps/kopete/logs",
        QDir::homePath() + "/.kde3/share/apps/kopete/logs"
    };
}

}
