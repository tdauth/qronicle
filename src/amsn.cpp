#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QtSql>
#include <QDateTime>
#include <QtConcurrent>

#include "amsn.hpp"

namespace qronicle {

QString Amsn::id() const {
    return "amsn";
}
    
Messenger::Messages Amsn::loadFile(const QString &filePath) {
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

    static QRegularExpression re(R"(\|\s*"?LGRA\[\|\s*"?LTIME(\d+)\s+\]\s*\|\s*"?LITA(.*?)\s?:\s*\|\s*"?LC([0-9A-F]{6})\s*(.*))", QRegularExpression::CaseInsensitiveOption);
    qint64 lineNumber = 1;
    auto myself = m_contacts.value("myself");
    auto myselfNick = myself.mfn;

    while (!in.atEnd()) {
        QString line = in.readLine();
        if (!line.isEmpty()) {
            QRegularExpressionMatch match = re.match(line);

            if (match.hasMatch()) {
                Message msg;
                msg.setProtocol("aMSN");
                msg.setMessenger("aMSN");
                msg.setFilePath(fileInfo.absoluteFilePath());
                msg.setLineNumber(lineNumber);
                
                qlonglong unixTime = match.captured(1).toLongLong();
                msg.setTimestamp(QDateTime::fromSecsSinceEpoch(unixTime));
                QString sender = match.captured(2);
                msg.setSourceNick(sender);
                
                if (m_contacts.contains(sender)) {
                    msg.setSource(m_contacts.value(sender).name);
                } else if (sender == myselfNick) {
                    msg.setSource(myself.mfn);
                } else {
                    msg.setSource(fileInfo.completeBaseName());
                }
                
                QString color = match.captured(3);
                QString message = match.captured(4);

                msg.setContent(message);
                msg.setContentHtml(QString("<font color='#%1'>%2</font>").arg(color, message));
                messages.append(std::move(msg));
            } else {
                qWarning() << "Ignoring aMSN log file line in file" << filePath << ":" << line;
            }
        }
        
        lineNumber++;
    }

    // set destinations
    for (Message &msg : messages) {
        if (msg.sourceNick() == myselfNick) {
            QString email = fileInfo.completeBaseName();
            msg.setDestination(email);
            
            if (msg.destinationNick().isEmpty() && m_contacts.contains(email)) {
                msg.setDestinationNick(m_contacts.value(email).nick);
            }
        } else {
            msg.setDestination(myself.login);
            msg.setDestinationNick(myselfNick);
        }
    }

    return messages;
}

Messenger::Messages Amsn::loadDirectories(const QStringList &dirPaths) {
    QStringList filePaths;

    for (const QString &dir : dirPaths) {
        QDirIterator it(dir, QStringList() << "*.log", QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            filePaths << it.next();
        }
    }

    if (filePaths.isEmpty()) {
        qWarning() << "No aMSN .log files found-";
        return {};
    }
    
    loadABooks(dirPaths);
    loadAvatars(dirPaths);

    qDebug() << "Loading" << filePaths.size() << "aMSN files in concurrently...";

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

QStringList Amsn::defaultDirectories() {
    return {
        QDir(QDir::home()).filePath(".amsn")
    };
}

void Amsn::loadABooks(const QStringList &dirPaths) {
    QStringList allABookFiles;
    
    for (const QString &basePath : dirPaths) {
        QDirIterator it(basePath, QStringList() << "abook.xml", QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            allABookFiles << it.next();
        }
    }
    
    qDebug() << "ABook files" << allABookFiles << "from" << dirPaths;

    
    if (!allABookFiles.isEmpty()) {
        QtConcurrent::blockingMappedReduced<Contacts>(
            allABookFiles,
            [](const QString &filePath) -> Contacts {
                Contacts localMap;
                QFile file(filePath);
                
                if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    qWarning() << "Could not open file:" << filePath << file.errorString();
                    return localMap;
                }
                
                // Avoid encoding errors like BOM markers etc.
                QString content = QString::fromUtf8(file.readAll());
                file.close();
                
                // Avoid error pseudo attribute:
                int headerEnd = content.indexOf("?>");
                if (headerEnd != -1) {
                    content = content.mid(headerEnd + 2);
                }
                
                QXmlStreamReader xml(content);
                Contact current;
                bool inContact = false;

                while (!xml.atEnd()) {
                    auto token = xml.readNext();

                    if (token == QXmlStreamReader::StartElement) {
                        auto tagName = xml.name();

                        if (tagName == "contact") {
                            inContact = true;
                            current = Contact();
                            current.name = xml.attributes().value("name").toString().toLower();
                        } else if (inContact) {
                            if (tagName == "cid") {
                                current.cid = xml.readElementText();
                            } else if (tagName == "nick") {
                                current.nick = xml.readElementText();
                            } else if (tagName == "displaypicfile") {
                                current.displaypicfile = xml.readElementText();
                            } else if (tagName == "mfn") {
                                current.mfn = xml.readElementText();
                            } else if (tagName == "login") {
                                current.login = xml.readElementText();
                            }
                        }
                    } 
                    else if (token == QXmlStreamReader::EndElement) {
                        if (xml.name() == "contact") {
                            if (!current.mfn.isEmpty()) {
                                localMap.insert(current.mfn, current);
                            }
                            
                            if (!current.login.isEmpty()) {
                                localMap.insert(current.login, current);
                            }
                            
                            if (!current.nick.isEmpty()) {
                                localMap.insert(current.nick, current);
                            }
                            
                            if (!current.name.isEmpty()) {
                                localMap.insert(current.name, current);
                            }
                            
                            inContact = false;
                            current = Contact();
                        }
                    }
                }
                
                if (xml.hasError()) {
                    qDebug() << "XML Error in" << filePath << ":" << xml.errorString();
                }
                return localMap;
            },
            [this](Contacts &/*dummy*/, const Contacts &intermediate) {
                //m_contacts.unite(intermediate);
                for (auto it = intermediate.constBegin(); it != intermediate.constEnd(); ++it) {
                    this->m_contacts.insert(it.key(), it.value());
                }
            }
        );
    }
    
    qDebug() << "ABook contacts" << m_contacts.size();
}

namespace {

struct ImageData {
    QString folderName;
    QString completeBaseName;
    QImage image;
};

}

void Amsn::loadAvatars(const QStringList &dirPaths) {
    qDebug() << "Avatars before" << m_avatars.size();
    QStringList allImageFiles;
    for (const QString &basePath : dirPaths) {
        QDirIterator it(basePath, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString path = it.next();
            if (path.contains("/displaypic/")) {
                allImageFiles << path;
            }
        }
    }

    if (!allImageFiles.isEmpty()) {
        QtConcurrent::blockingMappedReduced<Avatars>(
            allImageFiles,
            [](const QString &path) -> ImageData {
                ImageData data;
                QFileInfo info(path);
                data.folderName = info.dir().dirName();
                data.completeBaseName = info.completeBaseName();
                
                QImage img;
                if (img.load(path)) {
                    data.image = img;
                }
                return data;
            },
            [this](Avatars &/*dummy*/, const ImageData &data) {
                if (!data.image.isNull()) {
                    // avatar for myself
                    if (data.folderName == "displaypic") {
                        // TODO handle multiple "myself" entries if there are multiple folders by considering the current dir
                        if (m_contacts.contains("myself")) {
                            auto myself = m_contacts.value("myself");
                            
                            this->m_avatars.insert("myself", data.image);
                            this->m_avatars.insert(myself.mfn, data.image);
                            this->m_avatars.insert(myself.login, data.image);
                        }
                    } else {
                        this->m_avatars.insert(data.folderName, data.image);
                        
                        for (auto it = m_contacts.constBegin(); it != m_contacts.constEnd(); ++it) {
                            if (it->displaypicfile == data.completeBaseName) {
                                this->m_avatars.insert(it->name, data.image);
                                this->m_avatars.insert(it->nick, data.image);
                            }
                        }
                    }
                }
            }
        );
    }
    qDebug() << "Avatars after" << m_avatars.size();
}

}
