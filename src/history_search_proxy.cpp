#include <QGuiApplication>
#include <QClipboard>
#include <QSqlQuery>

#include "history_search_proxy.hpp"

namespace qronicle {

void HistorySearchProxy::copyToClipboard(const QString &text) {
    QGuiApplication::clipboard()->setText(text);
}    
    
int HistorySearchProxy::findIndexById(QVariant targetId) {
    qlonglong target = targetId.toLongLong();

    for (int i = 0; i < rowCount(); ++i) {
        QVariant v = data(index(i, 0), HistoryModel::MessageIdRole);
        
        if (v.toLongLong() == target) {
            return i;
        }
    }
    return -1;
}

int HistorySearchProxy::getUnfilteredIndex(int currentProxyRow) {
    // Mappt den sichtbaren Index (Proxy) auf den echten Index (SQL-Model)
    QModelIndex proxyIdx = index(currentProxyRow, 0);
    QModelIndex sourceIdx = mapToSource(proxyIdx);
    return sourceIdx.row();
}

int HistorySearchProxy::totalCount() const {
    auto *sqlModel = qobject_cast<QSqlTableModel*>(sourceModel());
    if (!sqlModel) return 0;

    // Wir fragen SQLite direkt nach der echten Anzahl aller Zeilen
    QSqlQuery query("SELECT COUNT(*) FROM messages", sqlModel->database());
    if (query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

int HistorySearchProxy::filteredCount() const {
    auto *sqlModel = qobject_cast<QSqlTableModel*>(sourceModel());
    if (!sqlModel) return 0;

    // Wenn kein Filter aktiv ist, ist filtered == total
    if (sqlModel->filter().isEmpty()) {
        return totalCount();
    }

    // Wenn ein Filter aktiv ist, zählen wir die Treffer mit der aktuellen WHERE-Klausel
    QString sql = QString("SELECT COUNT(*) FROM messages WHERE %1").arg(sqlModel->filter());
    QSqlQuery query(sql, sqlModel->database());
    if (query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

QString HistorySearchProxy::dateRange() const {
    auto *sqlModel = qobject_cast<QSqlTableModel*>(sourceModel());
    if (!sqlModel) return QString();

    QString filter = sqlModel->filter();
    QString whereClause = filter.isEmpty() ? "" : "WHERE " + filter;

    // Wir holen MIN und MAX direkt per SQL
    QSqlQuery query(QString("SELECT MIN(created_at), MAX(created_at) FROM messages %1").arg(whereClause), 
                    sqlModel->database());
    
    if (query.next() && !query.value(0).isNull()) {
        // 1. Aus SQLite (ISO-String) in QDateTime wandeln
        QDateTime start = QDateTime::fromString(query.value(0).toString(), Qt::ISODate);
        QDateTime end = QDateTime::fromString(query.value(1).toString(), Qt::ISODate);
        
        // 2. QLocale nutzen (formatiert automatisch nach Landessprache des Nutzers)
        QLocale locale;
        QString startStr = locale.toString(start.toLocalTime(), QLocale::ShortFormat);
        QString endStr = locale.toString(end.toLocalTime(), QLocale::ShortFormat);

        // 3. Übersetzbar zurückgeben
        return tr("%1 - %2", "Date range from - to")
            .arg(startStr)
            .arg(endStr);
    }
    return tr("-", "No date available");
}

void HistorySearchProxy::triggerFilter(const QString &f) {
    // 1. Zugriff auf das zugrunde liegende SQL-Model
    auto *sqlModel = qobject_cast<HistoryModel*>(sourceModel());
    if (sqlModel) {
        // 2. Die schwere Arbeit an SQLite übergeben
        sqlModel->applyFilters(
            m_filterFilePath, 
            m_filterMessage, 
            m_filterNick, 
            m_filterTarget,
            m_filterMessenger,
            m_filterProtocol
        );
    }
    
    // Invalidate ist hier nicht mehr für die Zeilenprüfung nötig, 
    // aber wir emittieren das Signal für die UI.
    emit filterChanged();
}

// WICHTIG: Die C++ Filterung ausschalten
bool HistorySearchProxy::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    // Da das SQL-Model nur noch Zeilen liefert, die dem Filter entsprechen,
    // lassen wir hier einfach alles durch (true).
    return true; 
}

}
