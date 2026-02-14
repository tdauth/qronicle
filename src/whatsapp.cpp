#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QtSql>
#include <QDateTime>

#include "whatsapp.hpp"

namespace chronicle {
    
namespace {
    
QString formatHtml(const QString &filePath, const QString &msg) {
    QString newContentHtml = msg;
                
    // URLs into HTML
    if (newContentHtml.contains("http://") || newContentHtml.contains("https://")) {
        static QRegularExpression urlRegex(R"((https?:\/\/[^\s\n\r]+))");
        newContentHtml.replace(urlRegex, R"(<a href="\1">\1</a>)");
    }
    
    // 1. Verzeichnis der aktuellen Datei ermitteln
    QString dirPath = QFileInfo(filePath).absolutePath(); 

    // 2. Regex für Anhänge definieren
    // Erkennt: BeliebigerDateiname.ext (Datei angehängt)
    static QRegularExpression fileRe("([^\\s]+) \\(Datei angehängt\\)");
    QRegularExpressionMatch fileMatch = fileRe.match(newContentHtml);

    if (fileMatch.hasMatch()) {
        QString fileName = fileMatch.captured(1); // z.B. "IMG-20251031-WA0000.jpg"
        
        // URL zusammenbauen (file:// Pfad)
        QString fileUrl = QString("<a href=\"%1\">%2</a>").arg(QUrl::fromLocalFile(dirPath + "/" + fileName).toString()).arg(fileName);
        
        // Den Text in der Nachricht ersetzen
        newContentHtml.replace(fileMatch.captured(0), fileUrl);
    }
    
    return newContentHtml;
}
    
}
    
QString WhatsApp::id() const {
    return "whatsapp";
}

Messenger::Messages WhatsApp::loadFile(const QString &filePath) {
    Messages messages;
    
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        
        QSet<QString> participants;
        
        while (!in.atEnd()) {
            QString line = in.readLine();
            static QRegularExpression re("^(\\d{2}\\.\\d{2}\\.\\d{2}, \\d{2}:\\d{2}) - ([^:]+): (.*)$");

            QRegularExpressionMatch match = re.match(line);

            if (match.hasMatch()) {
                // Neue Nachricht gefunden
                QString timestamp = match.captured(1);
                QString name      = match.captured(2);
                QString message   = match.captured(3);
                
                participants.insert(name); 

                Message msg;
                msg.setProtocol("whatsapp");
                msg.setSource(name);
                msg.setSourceNick(name);
                
                QDateTime dt = QDateTime::fromString(timestamp, "dd.MM.yy, HH:mm");
                
                // Falls das Jahr < 100 ist (z.B. 19), addiere 2000 Jahre
                if (dt.date().year() < 2000) {
                    dt = dt.addYears(100);
                }
                
                msg.setTimestamp(dt);
                msg.setContent(message);
                msg.setContentHtml(formatHtml(filePath, message));
                
                messages.append(msg);
            } else if (!messages.isEmpty()) {
                // Keine Übereinstimmung -> Gehört zur vorherigen Nachricht
                // Wir holen die letzte Nachricht per Referenz und hängen den Text an
                Message &lastMsg = messages.last();
                lastMsg.setContent(lastMsg.content() + "\n" + line);
                lastMsg.setContentHtml(lastMsg.contentHtml() + "\n" + formatHtml(filePath, line));
            } else {
                qWarning() << "Unexpected WhatsApp line with 0 lines:" << line << "in file" << filePath;
            }
        }
        
        for (Message &msg : messages) {
            if (msg.destination().isEmpty()) {
                QSet<QString> potentialDest = participants;
                potentialDest.remove(msg.source());

                if (!potentialDest.isEmpty()) {
                    QString other = *potentialDest.begin(); 
                    msg.setDestination(other);
                    msg.setDestinationNick(other);
                } else {
                    msg.setDestination("unknown");
                }
            }
        }
    }

    return messages;
}

Messenger::Messages WhatsApp::loadDirectory(const QString &dir) {
    Messages allMessages;
    qDebug() << "WhatsApp dir:" << dir;
    QDirIterator it(dir, QStringList() << "*.txt", QDir::Files, QDirIterator::Subdirectories);

    while (it.hasNext()) {
        QString filePath = it.next();

        qDebug() << "Loading WhatsApp file:" << filePath;
        
        Messages fileMessages = loadFile(filePath);
        if (!fileMessages.empty()) {
            allMessages.append(fileMessages);
        }
    }
    
    return allMessages;
}

QStringList WhatsApp::defaultDirectories() {
    return {};
}

}
