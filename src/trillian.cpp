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

#include "trillian.hpp"

namespace qronicle {

namespace {

QDateTime getTimestamp(const QString &time) {
    bool ok;
    qint64 msecs = time.toLongLong(&ok);
    if (!ok) {
        qDebug() << "Ungültiger Zeitstempel:" << time;
        return QDateTime();
    }
    // Falls Kopete Sekunden statt Millisekunden nutzt (1185831353 = 2007)
    // Wenn die Zahl klein ist, als Sekunden interpretieren:
    if (msecs < 10000000000LL) {
        return QDateTime::fromSecsSinceEpoch(msecs);
    }

    return QDateTime::fromMSecsSinceEpoch(msecs);
}

QString decodeMessage(const QString &message) {
    if (!message.contains('%')) {
        return message; // Schneller Ausstieg, falls nichts zu decodieren ist
    }
    // fromPercentEncoding wandelt %3C in <, %22 in " usw. um
    return QUrl::fromPercentEncoding(message.toUtf8());
}

}

QString Trillian::id() const {
    return "trillian";
}

Messenger::Messages Trillian::loadFile(const QString &filePath) {
    Messages messages;

    QFileInfo fileInfo(filePath);

    if (!fileInfo.exists()) {
        return messages;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return messages;
    }

    // Die gesamte Datei einlesen
    QByteArray data = file.readAll();

    QXmlStreamReader reader;
    // Wir "faken" ein Root-Element
    reader.addData("<root>");
    reader.addData(data);
    reader.addData("</root>");

    QMap<QString, QString> participantNicknames;

    QString protocol;
    QString owner;
    QString other;

    while (!reader.atEnd()) {
        reader.readNext();

        if (reader.isStartElement()) {
            QStringView tagName = reader.name();

            if (tagName == QStringLiteral("session")) {
                auto attrs = reader.attributes();

                if (attrs.value(QStringLiteral("type")).toString() == "start") {
                    owner = attrs.value(QStringLiteral("from")).toString();
                    other = attrs.value(QStringLiteral("to")).toString();
                    // TODO messenger and protocol are different
                    protocol = attrs.value(QStringLiteral("medium")).toString();
                }
            } else if (tagName == QStringLiteral("message")) {
                Message message;
                auto attrs = reader.attributes();

                // TODO messenger and protocol are different
                message.setProtocol(protocol);
                message.setMessenger("Trillian");
                message.setFilePath(fileInfo.absoluteFilePath());
                message.setLineNumber(reader.lineNumber());

                QString type = attrs.value(QStringLiteral("type")).toString();
                bool in = type == "incoming_privateMessage";

                QString rawValue = attrs.value(QStringLiteral("from_display")).toString();
                QString nick = decodeMessage(rawValue);
                message.setSourceNick(nick);

                message.setSource(attrs.value(QStringLiteral("from")).toString());
                message.setDestination(attrs.value(QStringLiteral("to")).toString());

                // Nickname-Cache
                if (!participantNicknames.contains(message.source())) {
                    participantNicknames.insert(message.source(), nick);
                }
                message.setDestinationNick(participantNicknames.value(message.destination()));

                message.setTimestamp(getTimestamp(attrs.value(QStringLiteral("time")).toString()));

                QString content = decodeMessage(attrs.value(QStringLiteral("text")).toString());
                message.setContent(content);

                message.setContentHtml(content); // Do not format HTML. It seems that Trillian already has HTML links.

                messages.push_back(std::move(message));
            }
        }
    }

    return messages;
}

Messenger::Messages Trillian::loadDirectories(const QStringList &dirPaths) {
    QStringList filePaths;

    // 1. Alle Verzeichnisse nach XML-Dateien durchsuchen
    for (const QString &dir : dirPaths) {
        QDirIterator it(dir, QStringList() << "*.xml", QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            filePaths << it.next();
        }
    }

    if (filePaths.isEmpty()) {
        qWarning() << "No Trillian XML files found in the given directories!";
        return {};
    }

    qDebug() << "Loading" << filePaths.size() << "Trillian files concurrently...";

    return QtConcurrent::blockingMappedReduced<Messenger::Messages>(
        filePaths,
        [this](const QString &path) {
            return loadFile(path);
        },
        [](Messenger::Messages &result, const Messenger::Messages &intermediate) {
            result.append(intermediate);
        },
        QtConcurrent::UnorderedReduce
    );
}

QStringList Trillian::defaultDirectories() {
    return {
        QDir::homePath() + "/.config/trillian"
    };
}

}
