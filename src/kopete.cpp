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

namespace qronicle {

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
    bool hasDocTypeKopeteHistory = false;
    QMap<QString, QString> participantNicknames;

    while (!reader.atEnd() && !reader.hasError()) {
        QXmlStreamReader::TokenType token = reader.readNext();
        //qDebug() << "Current Token:" << reader.tokenString() << "Name:" << reader.name();
        
        if (token == QXmlStreamReader::DTD) {
            QString dtdName = reader.dtdName().toString();
            
            if (dtdName != "Kopete-History") {
                qWarning() << "XML Error in" << filePath << " is missing DOCTYPE Kopete-History and has instead" << dtdName << "!";
                return messages;
            } else {
                hasDocTypeKopeteHistory = true;
                //qDebug() << "Is Kopete-History:" << filePath;
            }
        } else if (hasDocTypeKopeteHistory && token == QXmlStreamReader::StartElement) {
            QStringView tagName = reader.name();

            if (tagName == QStringLiteral("msg")) {
                //qDebug() << "Found Kopete message!";
                Message message;
                auto attrs = reader.attributes();

                message.setFilePath(fileInfo.absoluteFilePath());
                message.setLineNumber(reader.lineNumber());
                message.setProtocol(protocol);
                message.setMessenger("Kopete");
                message.setSource(attrs.value(QStringLiteral("from")).toString());
                message.setSourceNick(attrs.value(QStringLiteral("nick")).toString());

                if (attrs.value(QStringLiteral("in")) == QStringLiteral("1")) {
                    message.setDestination(owner);
                } else {
                    message.setDestination(partner);
                }

                if (!participantNicknames.contains(message.source())) {
                    participantNicknames.insert(message.source(), message.sourceNick());
                }
                message.setDestinationNick(participantNicknames.value(message.destination()));

                QString timeAttr = attrs.value(QStringLiteral("time")).toString();

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
    
    if (!hasDocTypeKopeteHistory) {
        qWarning() << "XML Error in" << filePath << " is missing DOCTYPE Kopete-History!";
    }

    return messages;
}

Messenger::Messages Kopete::loadDirectories(const QStringList &dirPaths) {
    QStringList filePaths;

    for (const QString &dir : dirPaths) {
        QDirIterator it(dir, QStringList() << "*.xml", QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            filePaths << it.next();
        }
    }

    if (filePaths.isEmpty()) {
        qWarning() << "Did not find any Kopete XML files in the given directories!";
        return {};
    }

    qDebug() << "Loading" << filePaths.size() << "Kopete files concurrently...";

    return QtConcurrent::blockingMappedReduced<Messenger::Messages>(
        filePaths,
        [this](const QString &path) {
            //qDebug() << "Loading Kopete XML file" << path;
            return loadFile(path);
        },
        [](Messenger::Messages &result, const Messenger::Messages &intermediate) {
            result.append(intermediate);
        },
        QtConcurrent::UnorderedReduce
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
