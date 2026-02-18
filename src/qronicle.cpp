#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QXmlStreamReader>
#include <QString>
#include <QStringView>
#include <QUtf8StringView>
#include <QStandardPaths>
#include <QtConcurrent>

#include "qronicle.hpp"

namespace qronicle {

QString Qronicle::id() const {
    return "qronicle";
}

Messenger::Messages Qronicle::loadFile(const QString &filePath) {
    Messages messages;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return messages;
    }

    QFileInfo fileInfo(filePath);
    QXmlStreamReader reader(&file);

    while (!reader.atEnd() && !reader.hasError()) {
        QXmlStreamReader::TokenType token = reader.readNext();

        if (token == QXmlStreamReader::StartElement) {
            if (reader.name() == QLatin1String("participant")) {
                QString nick = reader.attributes().value("nickName").toString();
                QString contact = reader.attributes().value("contact").toString();
                //qDebug() << "Teilnehmer:" << nick << "(" << contact << ")";
            }

            if (reader.name() == QLatin1String("message")) {
                Message message;
                auto attrs = reader.attributes();

                message.setFilePath(fileInfo.absoluteFilePath());
                message.setLineNumber(reader.lineNumber());
                message.setProtocol(attrs.value(QStringLiteral("protocol")).toString());
                message.setMessenger(id());
                message.setSource(attrs.value(QStringLiteral("from")).toString());
                message.setSourceNick(attrs.value(QStringLiteral("fromNickName")).toString());
                message.setDestination(attrs.value(QStringLiteral("to")).toString());
                message.setDestinationNick(attrs.value(QStringLiteral("toNickName")).toString());
                message.setTimestamp(QDateTime::fromString(attrs.value(QStringLiteral("timestamp")).toString(), Qt::ISODate));

                // Zum Body-Element springen, um den Text zu lesen
                reader.readNextStartElement();
                if (reader.name() == QLatin1String("body")) {
                    QString content = reader.readElementText();
                    message.setContent(content);
                    message.setContentHtml(content);
                }

                messages.push_back(std::move(message));
            }
        }
    }

    if (reader.hasError()) {
        qWarning() << "XML Error in" << filePath << ":" << reader.errorString();
    }

    return messages;
}

Messenger::Messages Qronicle::loadDirectories(const QStringList &dirPaths) {
    QStringList filePaths;

    for (const QString &dir : dirPaths) {
        QDirIterator it(dir, QStringList() << "*.xml", QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            filePaths << it.next();
        }
    }

    if (filePaths.isEmpty()) {
        qWarning() << "Did not find any qronicle XML files in the given directories!";
        return {};
    }

    qDebug() << "Loading" << filePaths.size() << "qronicle files concurrently...";

    return QtConcurrent::blockingMappedReduced<Messenger::Messages>(
        filePaths,
        [this](const QString &path) {
            // qDebug() << "Loading qronicle XML file" << path;
            return loadFile(path);
        },
        [](Messenger::Messages &result, const Messenger::Messages &intermediate) {
            result.append(intermediate);
        },
        QtConcurrent::UnorderedReduce
    );
}

QStringList Qronicle::defaultDirectories() {
    return {};
}

}
