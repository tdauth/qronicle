#include <QSqlQuery>
#include <QSqlError>
#include <QTimeZone>

#include "history_model.hpp"

namespace qronicle {

HistoryModel::HistoryModel(QSqlDatabase db) : QSqlTableModel(nullptr, db) {
    setTable("messages");
    // Wichtig: Daten müssen sofort geladen werden
    setSort(fieldIndex("created_at"), Qt::AscendingOrder);
    select();
}

QVariant HistoryModel::data(const QModelIndex& index, int role) const {
    if (role < Qt::UserRole) {
        return QSqlTableModel::data(index, role);
    }

    // Mapping deiner Custom Roles auf die Spalten der Datenbank
    const QSqlRecord rec = record(index.row());
    switch (role) {
        case MessageIdRole:   return rec.value("id").toLongLong();
        case FilePathRole:   return rec.value("filePath");
        case LineNumberRole: return rec.value("lineNumber").toLongLong();
        case ContentRole:    return rec.value("messageHtml"); // Nutze messageHtml für Content
        case SenderRole:     return rec.value("sender");
        case SenderNickRole: return rec.value("senderNick");
        case SenderAvatarRole: return rec.value("sender");
        case TargetRole:     return rec.value("receiver");
        case TargetNickRole: return rec.value("receiverNick");
        case TargetAvatarRole: return rec.value("receiver");
        case MessengerRole:  return rec.value("messenger");
        case ProtocolRole:   return rec.value("protocol");
        case OutRole:   return rec.value("out");
        case TimestampRole:  return rec.value("created_at").toDateTime();

        default: return {};
    }
}

QHash<int, QByteArray> HistoryModel::roleNames() const {
    return {
        {MessageIdRole, "messageId"},
        {FilePathRole, "filePath"},
        {LineNumberRole, "lineNumber"},
        {ContentRole, "messageText"},
        {TimestampRole, "time"},
        {SenderRole, "sourceId"},
        {SenderNickRole, "sourceNick"},
        {SenderAvatarRole, "sourceAvatar"},
        {TargetRole, "targetId"},
        {TargetNickRole, "targetNick"},
        {TargetAvatarRole, "targetAvatar"},
        {MessengerRole, "messenger"},
        {ProtocolRole, "protocol"},
        {OutRole, "out"},
    };
}

void HistoryModel::applyFilters(const QString &filePath, const QString &message, const QString &particpant,
                            const QString &sender, const QString &target,
                            const QString &messenger, const QString &protocol, const QDateTime &from, const QDateTime &to) {
    QStringList filters;

    qDebug() << "Apply filters with from and to" << from << to;

    if (!filePath.isEmpty()) {
        filters << QString("filePath LIKE '%%1%' COLLATE NOCASE").arg(filePath);
    }

    if (!message.isEmpty()) {
        filters << QString("messageHtml LIKE '%%1%' COLLATE NOCASE").arg(message);
    }

    if (!particpant.isEmpty()) {
        filters << QString("(senderNick LIKE '%%1%' OR receiverNick LIKE '%%2%') COLLATE NOCASE").arg(particpant).arg(particpant);
    }

    if (!sender.isEmpty()) {
        filters << QString("senderNick LIKE '%%1%' COLLATE NOCASE").arg(sender);
    }

    if (!target.isEmpty()) {
        filters << QString("receiverNick LIKE '%%1%' COLLATE NOCASE").arg(target);
    }

    if (!messenger.isEmpty()) {
        filters << QString("messenger LIKE '%%1%' COLLATE NOCASE").arg(messenger);
    }

    if (!protocol.isEmpty()) {
        filters << QString("protocol LIKE '%%1%' COLLATE NOCASE").arg(protocol);
    }

    if (from.isValid()) {
        QDateTime fromUtc = from.toUTC();
        QTime t = fromUtc.time();
        // cut seconds and milliseconds
        fromUtc.setTime(QTime(t.hour(), t.minute(), 0, 0));
        QString fromStr = fromUtc.toString(Qt::ISODate);
        qDebug() << "Filter from" << fromStr;
        filters << QString("datetime(created_at) >= datetime('%1')").arg(fromStr);
    }

    if (to.isValid()) {
        // add a second to ignore milliseconds
        QString toStr = to.addSecs(1).toUTC().toString(Qt::ISODate);
        qDebug() << "Filter to" << toStr;
        filters << QString("datetime(created_at) <= datetime('%1')").arg(toStr);
    }


    this->setFilter(filters.join(" AND "));
    if (!this->select()) {
        qDebug() << "SQL Error:" << this->lastError().text();
    }
}

QStringList HistoryModel::getAllMessengers() {
    QStringList messengers;
    QSqlQuery query(database());
    query.exec("SELECT messenger FROM view_messenger ORDER BY messenger ASC;");
    while (query.next()) {
        messengers << query.value(0).toString();
    }
    //qDebug() << "All messengers:" << messengers;
    return messengers;
}

QStringList HistoryModel::getAllProtocols() {
    QStringList protocols;
    QSqlQuery query(database());
    query.exec("SELECT protocol FROM view_protocols ORDER BY protocol ASC;");
    while (query.next()) {
        protocols << query.value(0).toString();
    }
    //qDebug() << "All protocols:" << protocols;
    return protocols;
}

QStringList HistoryModel::getAllNickNames() {
    QStringList nicks;
    QSqlQuery query(database());
    query.exec("SELECT nick FROM view_all_nicks ORDER BY nick ASC;");
    while (query.next()) {
        nicks << query.value(0).toString();
    }
    //qDebug() << "All nicks:" << nicks;
    return nicks;
}

QDateTime HistoryModel::getFrom() {
    QSqlQuery query(database());
    query.exec("SELECT MIN(datetime(created_at)) FROM messages;");

    QDateTime from; // Startet als "invalid" (null)

    if (query.next() && !query.value(0).isNull()) {
        QString raw = query.value(0).toString();
        from = QDateTime::fromString(raw, "yyyy-MM-dd HH:mm:ss");

        if (from.isValid()) {
            from.setTimeZone(QTimeZone::UTC);
        }
    }

    if (!from.isValid()) {
        qDebug() << "From: DB value invalid or empty, using current UTC as fallback";
        from = QDateTime::currentDateTimeUtc();
    }

    qDebug() << "Final From (UTC):" << from.toString(Qt::ISODate);

    return from;
}

QDateTime HistoryModel::getTo() {
    QSqlQuery query(database());
    query.exec("SELECT MAX(datetime(created_at)) FROM messages;");

    QDateTime to;

    if (query.next() && !query.value(0).isNull()) {
        QString raw = query.value(0).toString();
        to = QDateTime::fromString(raw, "yyyy-MM-dd HH:mm:ss");

        if (to.isValid()) {
            to.setTimeZone(QTimeZone::UTC);
        }
    }

    if (!to.isValid()) {
        qDebug() << "To: DB value invalid or empty, using current UTC as fallback";
        to = QDateTime::currentDateTimeUtc();
    }

    qDebug() << "Final To (UTC):" << to.toString(Qt::ISODate);

    return to;
}

int HistoryModel::rowsCount() const {
    QSqlQuery query(database());
    query.exec("SELECT COUNT(*) FROM messages");
    if (query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

}
