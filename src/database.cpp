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

    if (QFile::remove(dbPath)) {
        qDebug() << "Sucessfully deleted database.";

        initDb();
    } else {
        qDebug() << "Deleting database failed. Maybe database is still locked?";
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
            created_at, messenger, protocol, status, "out", "type", channel
        ) VALUES (
            ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?
        )
    )");

    QVariantList fPaths, lines, snds, sndNicks, rcvs, rcvNicks,
                 msgs, msgHtmls, times, mngrs, protos, stats, out, type, channel;

    for (const auto &m : messages) {
        fPaths   << m.filePath();
        lines    << m.lineNumber();
        snds     << m.source();
        sndNicks << m.sourceNick();
        rcvs     << m.destination();
        rcvNicks << m.destinationNick();
        msgs     << m.content();
        msgHtmls << m.contentHtml();
        times    << m.timestamp().toUTC().toString(Qt::ISODate);
        mngrs    << m.messenger();
        protos   << m.protocol();
        stats    << m.status();
        out      << m.out();
        type     << m.type();
        channel  << m.channel();
    }

    query.addBindValue(fPaths);
    query.addBindValue(lines);
    query.addBindValue(snds);
    query.addBindValue(sndNicks);
    query.addBindValue(rcvs);
    query.addBindValue(rcvNicks);
    query.addBindValue(msgs);
    query.addBindValue(msgHtmls);
    query.addBindValue(times);
    query.addBindValue(mngrs);
    query.addBindValue(protos);
    query.addBindValue(stats);
    query.addBindValue(out);
    query.addBindValue(type);
    query.addBindValue(channel);

    if (!query.execBatch()) {
        qDebug() << "Batch Insert Error:" << query.lastError().text();
        m_db.rollback();
    } else {
        m_db.commit();
        qDebug() << messages.size() << "messages stored.";
    }
}

void Database::applyAliases(QMap<QString, QString> &&aliases) {
    if (aliases.isEmpty()) return;

    m_db.transaction();

    QVariantList ids;
    QVariantList nicks;

    for (auto it = aliases.begin(); it != aliases.end(); ++it) {
        ids << it.key();
        nicks << it.value();
    }

    QSqlQuery query(m_db);

    query.prepare("UPDATE messages SET senderNick = ? WHERE sender = ?");
    query.addBindValue(nicks);
    query.addBindValue(ids);

    if (!query.execBatch()) {
        qDebug() << "Sender Update Batch Fehler:" << query.lastError().text();
        m_db.rollback();
        return;
    }

    query.prepare("UPDATE messages SET receiverNick = ? WHERE receiver = ?");
    query.addBindValue(nicks);
    query.addBindValue(ids);

    if (!query.execBatch()) {
        qDebug() << "Receiver Update Batch Fehler:" << query.lastError().text();
        m_db.rollback();
        return;
    }

    if (m_db.commit()) {
        qDebug() << "Aliases successfully applied to all messages.";
    } else {
        qDebug() << "Commit failed during alias update!";
        m_db.rollback();
    }
}

void Database::clearAllMessages() {
    qDebug() << "Clearing all messages from database...";

    if (!m_db.isOpen()) {
        qDebug() << "Database error: Connection is not open.";
        return;
    }

    QSqlQuery pragmaQuery(m_db);
    // 1. Schreibt extrem schnell in den RAM-Cache statt synchron auf die langsame Festplatte zu warten
    pragmaQuery.exec("PRAGMA synchronous = OFF");
    // 2. Nutzt das moderne WAL-Journal, welches Löschvorgänge massiv beschleunigt
    pragmaQuery.exec("PRAGMA journal_mode = WAL");

    m_db.transaction();
    QSqlQuery query(m_db);

    query.prepare("DELETE FROM messages");

    if (!query.exec()) {
        qDebug() << "Clear Messages Error:" << query.lastError().text();
        m_db.rollback();
    } else {
        m_db.commit();
        qDebug() << "All messages successfully cleared from database.";
    }

    // Sicherer Standardzustand für normale App-Nutzung wiederherstellen
    pragmaQuery.exec("PRAGMA synchronous = FULL");
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
            qDebug() << "Missing schema file!";
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
        qDebug() << "Successfully loaded database schema!";
    } else {
        qDebug() << "Database connection failed!";
    }
}

}
