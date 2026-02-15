#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QtSql>
#include <QDateTime>
#include <QtConcurrent>

#include "whatsapp.hpp"

namespace qronicle {
    
namespace {
    
QString formatAttachments(const QString &filePath, const QString &msg) {
    QString newContentHtml = msg;
                
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
    QFileInfo fileInfo(filePath);
    
    if (!fileInfo.exists()) {
        return messages;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return messages;
    }

    // Vorab-Reservierung schätzt die Anzahl der Nachrichten (Dateigröße / ca. 100 Zeichen pro Nachricht)
    messages.reserve(file.size() / 100);

    // Gesamte Datei in den Speicher lesen ist bei Logs meist schneller als zeilenweise
    // Falls Datei > 100MB, bleibe bei QTextStream, sonst:
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);

    QSet<QString> participants;
    // Regex einmalig außerhalb der Schleife kompilieren (schon erledigt durch static)
    static QRegularExpression re("^(\\d{2}\\.\\d{2}\\.\\d{2}, \\d{2}:\\d{2}) - ([^:]+): (.*)$");
    qint64 lineNumber = 1;

    while (!in.atEnd()) {
        QString line = in.readLine();
        if (!line.isEmpty()) {
            QRegularExpressionMatch match = re.match(line);

            if (match.hasMatch()) {
                Message msg;
                QString name = match.captured(2);
                
                // Vermeide unnötige String-Operationen, falls Name schon bekannt
                if (!participants.contains(name)) {
                    participants.insert(name);
                }

                msg.setProtocol("WhatsApp");
                msg.setMessenger("WhatsApp");
                msg.setFilePath(fileInfo.absoluteFilePath());
                msg.setLineNumber(lineNumber);
                msg.setSource(name);
                msg.setSourceNick(name);
                
                // Effizientes Datumsparshing
                QDateTime dt = QDateTime::fromString(match.captured(1), "dd.MM.yy, HH:mm");
                if (dt.date().year() < 2000) {
                    dt = dt.addYears(100);
                }
                
                msg.setTimestamp(dt);
                
                QString content = match.captured(3);
                msg.setContent(content);
                // ACHTUNG: formatHtml/formatAttachments sind oft teuer. 
                // Wenn möglich, erst beim Anzeigen (Lazy Loading) generieren!
                msg.setContentHtml(formatHtml(formatAttachments(filePath, content)));
                
                messages.append(std::move(msg));
            } else if (!messages.isEmpty()) {
                // Performance: Letzte Nachricht direkt bearbeiten
                Message &lastMsg = messages.last();
                
                // String-Builder (+=) ist in Qt optimiert, aber viele Appends kosten.
                QString newContent = lastMsg.content();
                newContent.append(u'\n').append(line);
                lastMsg.setContent(newContent);
                
                QString newHtml = lastMsg.contentHtml();
                newHtml.append(u'\n').append(formatHtml(formatAttachments(filePath, line)));
                lastMsg.setContentHtml(newHtml);
            }
        }
        
        lineNumber++;
    }

    // Destination-Logik optimieren
    if (participants.size() == 2) {
        QString p1 = *participants.begin();
        QString p2 = *(++participants.begin());

        for (Message &msg : messages) {
            const QString& other = (msg.source() == p1) ? p2 : p1;
            msg.setDestination(other);
            msg.setDestinationNick(other);
        }
    } else {
        // Fallback für Gruppen oder unbekannte Teilnehmer
        for (Message &msg : messages) {
            if (msg.destination().isEmpty()) {
                msg.setDestination("group/unknown");
            }
        }
    }

    return messages;
}

// Hilfsfunktion zum Zusammenführen der Ergebnisse (Reduce-Schritt)
void mergeMessages(Messenger::Messages &result, const Messenger::Messages &intermediate) {
    result.append(intermediate);
}

Messenger::Messages WhatsApp::loadDirectories(const QStringList &dirPaths) {
    QStringList allFilePaths;

    // 1. Alle Dateipfade aus allen Verzeichnissen sammeln
    for (const QString &dir : dirPaths) {
        QDirIterator it(dir, QStringList() << "*.txt", QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            allFilePaths << it.next();
        }
    }

    if (allFilePaths.isEmpty()) {
        return {};
    }

    qDebug() << "Loading" << allFilePaths.size() << "WhatsApp files in parallel from" << dirPaths.size() << "directories...";

    // 2. Parallelisiertes Laden (bleibt fast gleich, nutzt aber die gesammelte Liste)
    Messenger::Messages allMessages = QtConcurrent::blockingMappedReduced<Messenger::Messages>(
        allFilePaths,
        [this](const QString &path) { 
            return loadFile(path); 
        },
        [](Messenger::Messages &result, const Messenger::Messages &intermediate) {
            result.append(intermediate);
        },
        QtConcurrent::UnorderedReduce // Optimierung: Reihenfolge beim Zusammenführen egal
    );

    // 3. Hier wäre der ideale Platz für das Deduplizieren (Distinct machen)
    // Da Dateien aus verschiedenen Verzeichnissen oft Überschneidungen haben
    QSet<Message> distinctSet = QSet<Message>(allMessages.begin(), allMessages.end());
    
    return distinctSet.values();
}

QStringList WhatsApp::defaultDirectories() {
    return {};
}

}
