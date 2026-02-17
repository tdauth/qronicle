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
        case FilePathRole:   return rec.value("filePath");
        case LineNumberRole: return rec.value("lineNumber");
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

void HistoryModel::applyFilters(const QString &filePath, const QString &message, 
                            const QString &nick, const QString &target,
                            const QString &messenger, const QString &protocol) {
    QStringList filters;

    // SQL nutzt "LIKE" für Teilsuche. % ist der Platzhalter.
    if (!filePath.isEmpty())  filters << QString("filePath LIKE '%%1%' COLLATE NOCASE").arg(filePath);
    if (!message.isEmpty())   filters << QString("messageHtml LIKE '%%1%' COLLATE NOCASE").arg(message);
    if (!nick.isEmpty())      filters << QString("senderNick LIKE '%%1%' COLLATE NOCASE").arg(nick);
    if (!target.isEmpty())    filters << QString("receiverNick LIKE '%%1%' COLLATE NOCASE").arg(target);
    if (!messenger.isEmpty()) filters << QString("messenger LIKE '%%1%' COLLATE NOCASE").arg(messenger);
    if (!protocol.isEmpty())  filters << QString("protocol LIKE '%%1%' COLLATE NOCASE").arg(protocol);

    // Filter setzen und Datenbank neu abfragen
    this->setFilter(filters.join(" AND ")); 
    this->select(); 
}

}
