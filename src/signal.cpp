#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
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

Messenger::Messages Signal::loadFile(const QString &filePath) {
    Messages messages;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open Signal file:" << filePath;
        return messages;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8); // Stellt korrekte UTF-8 Darstellung sicher

    qint64 lineNumber = 0;

    // Zeile für Zeile einlesen, bis das Ende der Datei erreicht ist
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        lineNumber++;

        // Leere Zeilen (z. B. am Dateiende) einfach überspringen
        if (line.isEmpty()) {
            continue;
        }


        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8(), &error);

        if (doc.isNull()) {
            qWarning() << "JSON error in" << filePath << ":" << error.errorString();
            return messages;
        }

        QFileInfo fileInfo(filePath);
        QString absoluteFilePath = fileInfo.absoluteFilePath();
        QString absoluteDirPath = fileInfo.absolutePath();

        QJsonObject rootObj = doc.object();

        if (rootObj.contains("chatItem")) {
            QJsonObject chatItem = rootObj.value("chatItem").toObject();
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
            }

            if (chatItem.contains("standardMessage")) {
                QJsonObject standardMessage = chatItem.value("standardMessage").toObject();

                if (standardMessage.contains("text")) {
                    QJsonObject text = standardMessage.value("text").toObject();

                    if (text.contains("body")) {
                        QString content = text.value("body").toString();
                        msg.setContent(content);
                        msg.setContentHtml(content);
                    }
                }
            }

            //qDebug() << msg.timestamp() << msg.content();

            messages.append(msg);
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
        qDebug() << "Found no Signal JSON files in given dirs!";
        return {};
    }

    qDebug() << "Parsing" << jsonFiles.size() << "Signal JSON files concurrently...";

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
