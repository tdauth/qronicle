#include <QSqlQuery>

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
        {ProtocolRole, "protocol"}
    };
}

void HistoryModel::applyFilters(const QString &filePath, const QString &message, const QString &particpant,
                            const QString &sender, const QString &target,
                            const QString &messenger, const QString &protocol) {
    QStringList filters;
    
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

    this->setFilter(filters.join(" AND "));
    this->select();
}

QStringList HistoryModel::getAllNickNames() {
    QStringList nicks;
    QSqlQuery query(database());
    query.exec("SELECT nick FROM view_all_nicks ORDER BY nick ASC;");
    while (query.next()) {
        nicks << query.value(0).toString();
    }
    qDebug() << "All nicks:" << nicks;
    return nicks;
}

}
