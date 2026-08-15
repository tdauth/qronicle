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

QStringList HistorySearchProxy::getAllMessengers() {
    auto *sqlModel = qobject_cast<HistoryModel*>(sourceModel());
    if (sqlModel == nullptr) {
        return QStringList();
    }

    return sqlModel->getAllMessengers();
}

QStringList HistorySearchProxy::getAllNickNames() {
    auto *sqlModel = qobject_cast<HistoryModel*>(sourceModel());
    if (sqlModel == nullptr) {
        return QStringList();
    }

    return sqlModel->getAllNickNames();
}

QDateTime HistorySearchProxy::getFrom() {
    auto *sqlModel = qobject_cast<HistoryModel*>(sourceModel());
    if (sqlModel == nullptr) {
        return QDateTime();
    }

    return sqlModel->getFrom();
}

QDateTime HistorySearchProxy::getTo() {
    auto *sqlModel = qobject_cast<HistoryModel*>(sourceModel());
    if (sqlModel == nullptr) {
        return QDateTime();
    }

    return sqlModel->getTo();
}

int HistorySearchProxy::sourceModelCount() const {
    auto *sqlModel = qobject_cast<HistoryModel*>(sourceModel());
    if (sqlModel == nullptr) {
        return 0;
    }

    return sqlModel->rowsCount();
}

int HistorySearchProxy::mapRowFromSource(int sourceRow) {
    if (!sourceModel()) return -1;

    while (sourceModel()->rowCount() <= sourceRow && sourceModel()->canFetchMore(QModelIndex())) {
        sourceModel()->fetchMore(QModelIndex());
    }

    while (this->rowCount() < sourceModel()->rowCount() && this->canFetchMore(QModelIndex())) {
        this->fetchMore(QModelIndex());
    }

    QModelIndex sourceIndex = sourceModel()->index(sourceRow, 0);
    QModelIndex proxyIndex = mapFromSource(sourceIndex);

    return proxyIndex.isValid() ? proxyIndex.row() : -1;
}

void HistorySearchProxy::setSourceModel(QAbstractItemModel *sourceModel) {
    if (this->sourceModel()) {
        disconnect(this->sourceModel(), &QAbstractItemModel::rowsInserted, this, &HistorySearchProxy::sourceModelCountChanged);
        disconnect(this->sourceModel(), &QAbstractItemModel::rowsRemoved, this, &HistorySearchProxy::sourceModelCountChanged);
    }
    QSortFilterProxyModel::setSourceModel(sourceModel);
    if (sourceModel) {
        connect(sourceModel, &QAbstractItemModel::rowsInserted, this, &HistorySearchProxy::sourceModelCountChanged);
        connect(sourceModel, &QAbstractItemModel::rowsRemoved, this, &HistorySearchProxy::sourceModelCountChanged);
    }
    emit sourceModelCountChanged();
}

QStringList HistorySearchProxy::getAllProtocols() {
    auto *sqlModel = qobject_cast<HistoryModel*>(sourceModel());
    if (sqlModel == nullptr) {
        return QStringList();
    }

    return sqlModel->getAllProtocols();
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

void HistorySearchProxy::triggerFilter() {
    qDebug() << "Trigger filter";
    auto *sqlModel = qobject_cast<HistoryModel*>(sourceModel());
    if (sqlModel) {
        sqlModel->applyFilters(
            m_filterFilePath,
            m_filterMessage,
            m_filterParticipant,
            m_filterSender,
            m_filterTarget,
            m_filterMessenger,
            m_filterProtocol,
            m_filterFrom,
            m_filterTo
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
