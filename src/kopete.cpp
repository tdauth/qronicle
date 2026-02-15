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

#include "kopete.hpp"

namespace chronicle {

namespace {
    
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

QString decodeMessage(const QString &message) {
    return message.contains(QChar::ReplacementCharacter) 
                ? QString::fromLatin1(message.toLatin1()) 
                : message;
}
    
}

QString Kopete::id() const {
    return "kopete";
}
    
Messenger::Messages Kopete::loadFile(const QString &filePath) {
    Messages messages;
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return messages;
    }
    
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

    QXmlStreamReader reader(&file);
    QMap<QString, QString> participantNicknames;

    while (!reader.atEnd() && !reader.hasError()) {
        QXmlStreamReader::TokenType token = reader.readNext();

        if (token == QXmlStreamReader::StartElement) {
            QStringView tagName = reader.name();

            if (tagName == u"date") {
                // Jahr/Monat aus dem Header überschreiben, falls vorhanden
                auto attrs = reader.attributes();
                if (attrs.hasAttribute(u"year")) year = attrs.value(u"year").toInt();
                if (attrs.hasAttribute(u"month")) month = attrs.value(u"month").toInt();
            } 
            else if (tagName == u"msg") {
                Message message;
                auto attrs = reader.attributes();
                
                message.setProtocol(protocol);
                message.setMessenger("Kopete");
                QString nick = attrs.value(u"nick").toString();
                message.setSourceNick(nick);

                if (attrs.value(u"in") == u"1") {
                    message.setSource(partner);
                    message.setDestination(owner);
                } else {
                    message.setSource(owner);
                    message.setDestination(partner);
                }

                // Nickname-Cache
                if (!participantNicknames.contains(message.source())) {
                    participantNicknames.insert(message.source(), nick);
                }
                message.setDestinationNick(participantNicknames.value(message.destination()));
                
                QString timeAttr = attrs.value(u"time").toString();

                if (timeAttr.isEmpty()) {
                    // FALLBACK: Wenn kein Zeitstempel da ist, nimm das Datum aus dem Header
                    // oder die aktuelle Zeit, damit die Nachricht nicht im Jahr -4713 landet.
                    message.setTimestamp(QDateTime(QDate(year > 0 ? year : 2000, month > 0 ? month : 1, 1), QTime(0,0)));
                } else {
                    // Versuche den Zeitstempel normal zu parsen
                    message.setTimestamp(getTimestamp(year, month, timeAttr));
                }

                QString content = decodeMessage(reader.readElementText());
                message.setContent(content);
                
                message.setContentHtml(formatHtml(content));

                messages.push_back(std::move(message));
            }
        }
    }

    if (reader.hasError()) {
        qWarning() << "XML Error in" << filePath << ":" << reader.errorString();
    }

    return messages;
}

Messenger::Messages Kopete::loadDirectories(const QStringList &dirPaths) {
    QStringList filePaths;

    // 1. Alle Verzeichnisse nach XML-Dateien durchsuchen
    for (const QString &dir : dirPaths) {
        QDirIterator it(dir, QStringList() << "*.xml", QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            filePaths << it.next();
        }
    }

    if (filePaths.isEmpty()) {
        qWarning() << "Keine Kopete XML Dateien in den angegebenen Verzeichnissen gefunden!";
        return {};
    }

    qDebug() << "Loading" << filePaths.size() << "Kopete files in parallel...";

    // 2. Parallel laden und direkt reduzieren (zusammenführen)
    // blockingMappedReduced ist eleganter als blockingMapped + manuelle Schleife
    return QtConcurrent::blockingMappedReduced<Messenger::Messages>(
        filePaths, 
        [this](const QString &path) {
            // qDebug() << "Loading Kopete XML file" << path;
            return loadFile(path); 
        },
        // Reduce-Funktion: Schiebt die Ergebnisse thread-sicher zusammen
        [](Messenger::Messages &result, const Messenger::Messages &intermediate) {
            // Falls du die Gesamtgröße kennst, könntest du hier result.reserve machen,
            // aber Qt macht das intern bei append für QList/QVector meist schon effizient.
            result.append(intermediate);
        },
        QtConcurrent::UnorderedReduce // Performance-Boost: Reihenfolge egal
    );
}

QStringList Kopete::defaultDirectories() {
    return {
        QDir::homePath() + "/.local/share/kopete",
        QDir::homePath() + "/.kde/share/apps/kopete",
        QDir::homePath() + "/.kde4/share/apps/kopete",
        QDir::homePath() + "/.kde3/share/apps/kopete"
    };
}

}
