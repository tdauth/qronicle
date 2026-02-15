#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QtSql>
#include <QDateTime>
#include <QtConcurrent>

#include "skype.hpp"

namespace chronicle {
    
namespace {
    
QByteArray loadAvatarFromDb(const QByteArray &data) {
    int start = data.indexOf("\xFF\xD8"); // Suche nach dem JPEG-Start
    if (start != -1) {
        return data.mid(start);
    }
    
    return data;
}

QString replaceSmileys(const QString &text) {
    QString result = text;
    // Skype smileys:
    static QRegularExpression smileyRegex("<ss type=\"([^\"]+)\">(.*?)</ss>");
    // 1. Alle Matches sammeln
    auto i = smileyRegex.globalMatch(result);
    QList<QRegularExpressionMatch> matches;
    
    while (i.hasNext()) {
        matches.append(i.next());
    }

    for (int j = matches.count() - 1; j >= 0; --j) {
        QRegularExpressionMatch match = matches.at(j);
        QString type = match.captured(1);
        QString originalText = match.captured(2);
    
        QString checkPath = QString(":/icons/skype/%1").arg(type);

        if (QFile::exists(checkPath)) {
            QString imgTag = QString("<img src=\"qrc:/icons/skype/%1\" width=\"20\" height=\"20\" align=\"middle\">")
                            .arg(type);
            
            result.replace(match.capturedStart(0), match.capturedLength(0), imgTag);
        } else {
            // Fallback auf Text
            result.replace(match.capturedStart(0), match.capturedLength(0), originalText);
            //qWarning() << "Skype icon does not exist:" << checkPath;
        }
    }
    
    return result;
}

}
    
QString Skype::id() const {
    return "skype";
}

Messenger::Messages Skype::loadFile(const QString &filePath) {
    Messages messages;
    
    QFileInfo dbInfo(filePath);
    QString mySkypeId = dbInfo.dir().dirName();

    const QString connectionName = QString("skype_con_%1_%2")
        .arg(quintptr(QThread::currentThreadId()))
        .arg(qHash(filePath)); // Einfacher globaler Aufruf von qHash
    
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        db.setDatabaseName(filePath);

        if (!db.open()) {
            qWarning() << "Error opening Skype DB:" << db.lastError().text();
            return messages;
        }

        QSqlQuery query(db);

        if (!query.prepare(R"(
            SELECT 
                m.body_xml, 
                m.timestamp, 
                m.author AS sender_id,
                m.from_dispname AS sender_display_name,
                m.dialog_partner AS receiver_id,
                c2.displayname AS receiver_display_name,
                c1.avatar_image AS sender_avatar,
                c2.avatar_image AS receiver_avatar
            FROM Messages m
            LEFT JOIN Contacts c1 ON m.author = c1.skypename
            LEFT JOIN Contacts c2 ON m.dialog_partner = c2.skypename
            ORDER BY m.timestamp ASC
        )")) {
            qWarning() << "Error preparing statement for Skype DB:" << db.lastError().text();
            return messages;
        }

        if (query.exec()) {
            while (query.next()) {
                Message msg;
                msg.setProtocol("Skype");
                msg.setMessenger("Skype");
                
                QString sender_id = query.value("sender_id").toString();
                QString sender_display_name = query.value("sender_display_name").toString();
                QString receiver_id = query.value("receiver_id").toString();
                QString receiver_display_name = query.value("receiver_display_name").toString();
                
                if (!m_avatars.contains(sender_id)) {
                    QByteArray senderAvatar = loadAvatarFromDb(query.value("sender_avatar").toByteArray());
                    QImage img;
                    
                    if (!senderAvatar.isEmpty()) {
                        if (!img.loadFromData(senderAvatar)) {
                            qDebug() << "Skype avatar image could not be loaded for:" << sender_id;
                        } else {
                            qDebug() << "Skype avatar image works for:" << sender_id;
                        }
                    } else {
                        qDebug() << "Skype avatar image is empty for:" << sender_id;
                    }
                    
                    m_avatars.insert(sender_id, img);
                }
                
                if (!m_avatars.contains(receiver_id)) {
                    QByteArray receiverAvatar = loadAvatarFromDb(query.value("receiver_avatar").toByteArray());
                    QImage img;
                    
                    if (!receiverAvatar.isEmpty()) {
                        if (!img.loadFromData(receiverAvatar)) {
                            qDebug() << "Skype avatar image could not be loaded for:" << receiver_id;
                        } else {
                            qDebug() << "Skype avatar image works for:" << receiver_id;
                        }
                    } else {
                        qDebug() << "Skype avatar image is empty for:" << receiver_id;
                    }
                    
                    m_avatars.insert(receiver_id, img);
                }
                
                if (receiver_display_name.isEmpty()) {
                    receiver_display_name = receiver_id;
                }
                
                msg.setSource(sender_id);
                msg.setSourceNick(sender_display_name);
                msg.setDestination(receiver_id);
                msg.setDestinationNick(receiver_display_name);
                msg.setTimestamp(QDateTime::fromSecsSinceEpoch(query.value("timestamp").toLongLong()));
                QString body_xml = query.value("body_xml").toString();
                msg.setContent(body_xml);
                QString contentHtml = replaceSmileys(body_xml);
                msg.setContentHtml(contentHtml);

                messages.append(msg);
            }
        } else {
            qWarning() << "SQL error:" << query.lastError().text();
        }

        db.close();
    }
    
    QSqlDatabase::removeDatabase(connectionName);

    return messages;
}

Messenger::Messages Skype::loadDirectories(const QStringList &dirPaths) {
    QStringList filePaths;

    // 1. Alle Pfade sammeln und filtern
    for (const QString &dirPath : dirPaths) {
        QDirIterator it(dirPath, QStringList() << "main.db", QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString path = it.next();
            QFileInfo info(path);
            QString skypeId = info.dir().dirName();

            // Deine Filter-Logik
            if (skypeId == "Data" || skypeId == "Content") {
                continue;
            }
            filePaths << path;
        }
    }

    if (filePaths.isEmpty()) return {};

    qDebug() << "Loading" << filePaths.size() << "Skype databases in parallel...";

    // 2. Parallelisiertes Laden und Reduzieren
    return QtConcurrent::blockingMappedReduced<Messenger::Messages>(
        filePaths,
        [this](const QString &filePath) {
            qDebug() << "Processing Skype DB:" << filePath;
            return loadFile(filePath); // Wichtig: loadFile muss thread-sicher sein!
        },
        [](Messenger::Messages &result, const Messenger::Messages &intermediate) {
            result.append(intermediate);
        },
        QtConcurrent::UnorderedReduce
    );
}

QStringList Skype::defaultDirectories() {
    return {};
}

}
