#include <QStandardPaths>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>

#include "database.hpp"

namespace qronicle {
 
Database::Database() {
    initDb();
}

Database::~Database() {
    // Im Destruktor deiner Klasse
    if (m_db.isOpen()) {
        m_db.close();
    }
}

void Database::removeDatabaseFile() {
    if (m_db.isOpen()) {
        m_db.close();
    }
    
    QString dbPath = m_db.databaseName();
    QString connectionName = m_db.connectionName();

    m_db.close();
    m_db = QSqlDatabase();
    QSqlDatabase::removeDatabase(connectionName);

    // Jetzt ist die Datei frei zum Löschen
    if (QFile::remove(dbPath)) {
        qDebug() << "Datenbankdatei erfolgreich gelöscht.";
        
        initDb();
    } else {
        qDebug() << "Löschen fehlgeschlagen. Ist die Datei noch gesperrt?";
    }
}

void Database::saveMessages(const Messenger::Messages &messages) {
    qDebug() << "Saving messages" << messages.size();
    if (messages.isEmpty()) {
        return;
    }

    m_db.transaction();

    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT INTO messages (
            filePath, lineNumber, sender, senderNick, 
            receiver, receiverNick, message, messageHtml, 
            created_at, messenger, protocol, status
        ) VALUES (
            :fPath, :line, :snd, :sndNick, 
            :rcv, :rcvNick, :msg, :msgHtml, 
            :time, :mngr, :proto, :stat
        )
    )");

    for (auto m : messages) {
        query.bindValue(":fPath", m.filePath());
        query.bindValue(":line",  m.lineNumber());
        query.bindValue(":snd",   m.source());
        query.bindValue(":sndNick", m.sourceNick());
        query.bindValue(":rcv",   m.destination());
        query.bindValue(":rcvNick", m.destinationNick());
        query.bindValue(":msg",   m.content());
        query.bindValue(":msgHtml", m.contentHtml());
        
        // Zeitstempel als ISO-String (wichtig für SQLite DATETIME)
        query.bindValue(":time",  m.timestamp().toString(Qt::ISODate));
        
        query.bindValue(":mngr",  m.messenger());
        query.bindValue(":proto", m.protocol());
        query.bindValue(":stat",  m.status());

        if (!query.exec()) {
            qDebug() << "Insert fehlgeschlagen für" << m.filePath() << ":" << query.lastError().text();
            m_db.rollback();
            return; 
    }
    }

    // 3. Alles auf einmal auf die Festplatte schreiben
    if (m_db.commit()) {
        qDebug() << "Stored messages in database successfully";
    } else {
        qDebug() << "Commit failed, rollback!";
        m_db.rollback();
    }
}

void Database::applyAliases(QMap<QString, QString> &&aliases) {
    if (aliases.isEmpty()) {
        return;
    }

    QSqlQuery query(m_db);
    
    // 1. Transaktion starten (EXTREM wichtig bei 800k Zeilen!)
    m_db.transaction();

    // 2. SQL vorbereiten
    // Wir überschreiben senderNick, wenn der sender mit der ID im Alias übereinstimmt
    query.prepare("UPDATE messages SET senderNick = :nick WHERE sender = :id");

    for (auto it = aliases.begin(); it != aliases.end(); ++it) {
        query.bindValue(":nick", it.value());
        query.bindValue(":id", it.key());
        
        if (!query.exec()) {
            qDebug() << "Update Fehler bei ID" << it.key() << ":" << query.lastError().text();
        }
    }

    // Das gleiche für die Empfänger (Receiver)
    query.prepare("UPDATE messages SET receiverNick = :nick WHERE receiver = :id");
    for (auto it = aliases.begin(); it != aliases.end(); ++it) {
        query.bindValue(":nick", it.value());
        query.bindValue(":id", it.key());
        query.exec();
    }

    // 3. Alles auf einmal auf die Festplatte schreiben
    m_db.commit();
}

void Database::initDb() {
    m_db = QSqlDatabase::addDatabase("QSQLITE", "qronicle");
    
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir configDir;
    if (!configDir.exists(configPath)) {
        configDir.mkpath(configPath);
    }

    QString fullDbPath = configPath + QDir::separator() + "qronicle.sqlite";
    qDebug() << "Creating SQLite database" << fullDbPath;
    m_db.setDatabaseName(fullDbPath);

    if (m_db.open()) {
        qDebug() << "Init db";
        // 1. Datei aus den Ressourcen laden
        QFile file(":/sql/schema.sql");
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "Schema-Datei nicht gefunden!";
            return;
        }

        QTextStream in(&file);
        QString sqlContent = in.readAll();
        file.close();

        // 2. Befehle am Semikolon trennen
        // Hinweis: Das ist eine einfache Methode. Komplexere Skripte mit 
        // Triggern benötigen einen robusteren Parser.
        QStringList queries = sqlContent.split(";", Qt::SkipEmptyParts);

        QSqlQuery query(m_db);
        m_db.transaction(); // Alles in einer Transaktion für Speed & Sicherheit

        for (const QString &sql : queries) {
            QString trimmedSql = sql.trimmed();
            qDebug() << "Executing SQL" << trimmedSql;
            if (trimmedSql.isEmpty()) {
                continue;
            }

            if (!query.exec(trimmedSql)) {
                qDebug() << "Fehler im Schema:" << query.lastError().text();
                m_db.rollback();
                return;
            }
        }

        m_db.commit();
        qDebug() << "Datenbank-Schema erfolgreich geladen!";
    } else {
        qDebug() << "Verbindung fehlgeschlagen!";
    }
}
    
}
