#include <optional>

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

#include "psi.hpp"

namespace qronicle {

namespace {

inline QString accountsFilePath(const QString &filePath) {
    QFileInfo fileInfo(filePath);
    QDir dir = fileInfo.absoluteDir();
    QString path = QDir::cleanPath(dir.absoluteFilePath("../accounts.xml"));
    QFileInfo check(path);

    if (check.exists()) {
        return check.canonicalFilePath();
    }

    return path;
}

inline QString vCardFilePath(const QString &filePath) {
    QFileInfo fileInfo(filePath);
    QDir dir = fileInfo.absoluteDir();
    QString rawPath = dir.absoluteFilePath("../vcard/" + fileInfo.completeBaseName() + ".xml");
    QString cleanPath = QDir::cleanPath(rawPath);
    QFileInfo check(cleanPath);

    if (check.exists()) {
        return check.canonicalFilePath();
    }

    return cleanPath;
}

}

QString Psi::id() const {
    return "psi";
}

Messenger::Messages Psi::loadFile(const QString &filePath) {
    Messages messages;
    QFileInfo fileInfo(filePath);

    if (fileInfo.suffix() == "xml" || fileInfo.suffix() == "history") {
        QString accountsFile = accountsFilePath(filePath);
        QString vCardFile = vCardFilePath(filePath);
        auto owner = matchingAccount(accountsFile);
        auto ownerCv = owner ? matchingCv(owner->vCardFilePath) : std::nullopt;
        auto cv = matchingCv(vCardFile);

        /*
        if (owner) {
            qDebug() << "Owner CV:" << owner->vCardFilePath;

            if (ownerCv) {
                qDebug() << "Owner CV nick:" << ownerCv->nickName;
            }
        }

        if (cv) {
            qDebug() << "Found CV for" << filePath << "with nick name" << cv->nickName;
        }
        */

        if (fileInfo.suffix() == "xml") {
            qDebug() << "XML files for PSI are not supported yet:" << filePath; // TODO Support XML files. Currently, I have no real examples.
        } else if (fileInfo.suffix() == "history") {
            QString protocol = "XMPP";

            if (fileInfo.fileName().contains("at_icq)")) {
                protocol = "ICQ";
            }

            // 12345_at_icq.jabber.fh%2dstralsund.de.history
            // cdauth_at_cdauth.de.history
            QString baseName = fileInfo.completeBaseName();
            QString result = baseName.section('_', 0, 0);
            QString partner = result;

            QFile file(filePath);

            // Datei im Lesemodus öffnen
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                qWarning() << "COuld not open Psi file:" << filePath;
                return messages;
            }

            QTextStream in(&file);
            in.setEncoding(QStringConverter::Utf8);
            qint64 lineNumber = 1;

            while (!in.atEnd()) {
                QString line = in.readLine();

                if (!line.isEmpty()) {
                    QStringList parts = line.split('|');

                    if (parts.size() >= 6) {
                        Message message;
                        message.setFilePath(fileInfo.absoluteFilePath());
                        message.setLineNumber(lineNumber);
                        //qDebug() << "Psi absolute file path:" << message.filePath();
                        message.setProtocol(protocol);
                        message.setMessenger("Psi");

                        QString nick = parts.at(4);

                        if (parts.at(3) == "from") {
                            message.setSource(partner);
                            message.setSourceNick(nick);

                            if (cv) {
                                if (!cv->nickName.isEmpty()) {
                                    message.setSourceNick(cv->nickName);
                                } else if (!cv->fn.isEmpty()) {
                                    message.setSourceNick(cv->fn);
                                }
                            }

                            if (owner) {
                                message.setDestination(owner->jid);

                                if (ownerCv && !ownerCv->nickName.isEmpty()) {
                                    message.setDestinationNick(ownerCv->nickName);
                                } else {
                                    message.setDestinationNick(owner->name);
                                }
                            } else {
                                message.setDestination("Unknown");
                            }



                        } else {
                            if (owner) {
                                message.setSource(owner->jid);

                                if (ownerCv && !ownerCv->nickName.isEmpty()) {
                                    message.setSourceNick(ownerCv->nickName);
                                } else {
                                    message.setSourceNick(owner->name);
                                }
                            } else {
                                message.setSource("Unknown");
                            }

                            message.setDestination(partner);
                            message.setDestinationNick(nick);

                            if (cv) {
                                if (!cv->nickName.isEmpty()) {
                                    message.setDestinationNick(cv->nickName);
                                } else if (!cv->fn.isEmpty()) {
                                    message.setDestinationNick(cv->fn);
                                }
                            }
                        }

                        message.setTimestamp(QDateTime::fromString(parts.at(1), Qt::ISODate));

                        QString content = parts.at(5);
                        message.setContent(content);
                        message.setContentHtml(formatHtml(content));

                        messages.push_back(std::move(message));
                    }
                }

                lineNumber++;
            }

            file.close();
        }
    } else {
        qWarning() << "Unsupported suffix for" << filePath;
    }

    return messages;
}

Messenger::Messages Psi::loadDirectories(const QStringList &dirPaths) {
    QStringList filePaths;

    for (const QString &dir : dirPaths) {
        QDirIterator it(dir, QStringList() << "*.xml" << "*.history", QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString path = it.next();
            QFileInfo fileInfo(path);
            if (fileInfo.dir().dirName() == "history") {
                filePaths << path;
            }
        }
    }

    if (filePaths.isEmpty()) {
        qWarning() << "No XML or .history files found for Psi!";
        return {};
    }

    qDebug() << "Loading PSI accounts and vCards once sequentially before";
    QSet<QString> processed;

    for (auto &filePath : filePaths) {
        QFileInfo fileInfo(filePath);
        QString accountsFile = accountsFilePath(filePath);
        QString vCardFile = vCardFilePath(filePath);

        if (!accountsFile.isEmpty() && !processed.contains(accountsFile)) {
            readAccountsFile(accountsFile);
            processed.insert(accountsFile);
        }

        if (!vCardFile.isEmpty() && !processed.contains(vCardFile)) {
            readVCardFile(vCardFile);
            processed.insert(vCardFile);
        }
    }

    qDebug() << "Loading" << filePaths.size() << "Psi files in parallel...";

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

QStringList Psi::defaultDirectories() {
    return {
        QDir(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)).filePath("psi+"),
        QDir(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)).filePath("psi")
    };
}

void Psi::readVCardFile(const QString &filePath) {
    qDebug() << "Loading Psi VCard file:" << filePath;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Unable to open Psi VCard file:" << filePath;

        return;
    }

    CV cv;
    QXmlStreamReader reader(&file);

    while (!reader.atEnd() && !reader.hasError()) {
        QXmlStreamReader::TokenType token = reader.readNext();

        if (token == QXmlStreamReader::StartElement) {
            QStringView tagName = reader.name();

            if (tagName == QStringLiteral("FN")) {
                cv.fn = reader.readElementText();
            } else if (tagName ==  QStringLiteral("NICKNAME")) {
                cv.nickName = reader.readElementText();
            } else if (tagName == QStringLiteral("PHOTO")) {
                QByteArray base64Data;

                // Lese weiter, bis das schließende </PHOTO> erreicht ist
                while (!(reader.isEndElement() && reader.name() == QStringLiteral("PHOTO")) && !reader.atEnd()) {
                    reader.readNext();

                    if (reader.isStartElement()) {
                        if (reader.name() == QStringLiteral("BINVAL")) {
                            // Den Base64-Text direkt in ein QByteArray lesen
                            base64Data = reader.readElementText().toLatin1();
                        }
                    }
                }

                if (!base64Data.isEmpty()) {
                    QByteArray imageRaw = QByteArray::fromBase64(base64Data);
                    QImage img;

                    if (!imageRaw.isEmpty()) {
                        if (!img.loadFromData(imageRaw)) {
                            //qDebug() << "Psi avatar image could not be loaded for:" << cv.fn;
                        } else {
                            qDebug() << "Psi avatar image works for:" << cv.fn;
                        }
                    } else {
                        //qDebug() << "Psi avatar image is empty for:" << cv.fn;
                    }

                    m_avatars.insert(cv.fn, img);
                    m_avatars.insert(cv.nickName, img);
                }
            }
        }
    }

    if (reader.hasError()) {
        qWarning() << "XML Error in" << filePath << ":" << reader.errorString();
    }

    m_cvs.insert(filePath, std::move(cv));
}

void Psi::readAccountsFile(const QString &filePath) {
    qDebug() << "Loading Psi accounts file:" << filePath;

    QFileInfo fileInfo(filePath);

    if (!fileInfo.exists()) {
        qDebug() << "Unable to open Psi accounts file:" << filePath;

        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Unable to open Psi accounts file:" << filePath;

        return;
    }

    Accounts accounts;
    QXmlStreamReader reader(&file);

    while (!reader.atEnd() && !reader.hasError()) {
        QXmlStreamReader::TokenType token = reader.readNext();

        if (token == QXmlStreamReader::StartElement) {
            QStringView tagName = reader.name();

            if (tagName == QStringLiteral("account") || tagName == QStringLiteral("a0") || tagName == QStringLiteral("a1")) {
                Account a;

                // Lese weiter, bis das schließende </PHOTO> erreicht ist
                while (!reader.atEnd()) {
                    reader.readNext();

                    if (reader.isStartElement()) {
                        if (reader.name() == QStringLiteral("jid")) {
                            a.jid = reader.readElementText();
                        } else if (reader.name() == QStringLiteral("name")) {
                            a.name = reader.readElementText();
                        } else if (reader.name() == QStringLiteral("host")) {
                            a.host = reader.readElementText();
                        }
                    }
                }

                if (!a.jid.isEmpty()) {
                    // TODO Last one wins.
                    accounts.insert(filePath, a);
                }
            }
        }
    }

    if (reader.hasError()) {
        qWarning() << "XML Error in" << filePath << ":" << reader.errorString();
    }

    // Load matching vCards for accounts:
    QString dirPath = fileInfo.absolutePath();

    for (auto it = accounts.begin(); it != accounts.end(); ++it) {
        Account &v = it.value();

        if (!v.jid.isEmpty()) {
            QString jidSafe = QString(v.jid).replace('@', "_at_"); // tamino@cdauth.de and the vcard name could be tamino_at_cdauth.de.xml
            QString vCardPath0 = dirPath + "/vcard/" + jidSafe + ".xml";
            QString vCardPath1 = dirPath + "/vcard/" + v.jid + ".xml";

            qDebug() << "Trying accounts vcards" << vCardPath0 << vCardPath1;

            if (QFileInfo(vCardPath0).exists()) {
                qDebug() << "Existing Psi vCards file from account" << vCardPath0;
                readVCardFile(vCardPath0);
                v.vCardFilePath = vCardPath0;
            } else if (QFileInfo(vCardPath1).exists()) {
                qDebug() << "Existing Psi vCards file from account" << vCardPath1;
                readVCardFile(vCardPath1);
                v.vCardFilePath = vCardPath1;
            }
        }

    }

    m_accounts.insert(std::move(accounts));
}

std::optional<Psi::CV> Psi::matchingCv(const QString &filePath) {
    if (m_cvs.isEmpty()) {
        return std::nullopt;
    }

    auto it = m_cvs.find(filePath);
    if (it == m_cvs.end()) {
        return std::nullopt;
    }
    return it.value();
}

std::optional<Psi::Account> Psi::matchingAccount(const QString &filePath) {
    if (m_accounts.isEmpty()) {
        return std::nullopt;
    }

    auto it = m_accounts.find(filePath);
    if (it == m_accounts.end()) {
        return std::nullopt;
    }
    return it.value();
}

}
