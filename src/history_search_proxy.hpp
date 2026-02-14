#ifndef CHRONICLE_HISTORY_SEARCH_PROXY_HPP
#define CHRONICLE_HISTORY_SEARCH_PROXY_HPP

#include <QSortFilterProxyModel>

#include "history_model.hpp"

namespace chronicle {

// Faster searches in GUI without using regexs.
class HistorySearchProxy : public QSortFilterProxyModel {
    Q_OBJECT
    
    // Properties für QML
    Q_PROPERTY(QString filterMessage READ filterMessage WRITE setFilterMessage NOTIFY filterChanged)
    Q_PROPERTY(QString filterNick READ filterNick WRITE setFilterNick NOTIFY filterChanged)
    
public:
    using QSortFilterProxyModel::QSortFilterProxyModel;
    
    // Getter & Setter
    QString filterMessage() const { return m_filterMessage; }
    void setFilterMessage(const QString &f) { m_filterMessage = f; invalidateFilter(); emit filterChanged(); }

    QString filterNick() const { return m_filterNick; }
    void setFilterNick(const QString &f) { m_filterNick = f; invalidateFilter(); emit filterChanged(); }

signals:
    void filterChanged();
    
protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override {
        auto *source = static_cast<HistoryModel*>(sourceModel());
        bool matches = true;
        
        // Logik: Alle gesetzten Filter müssen gleichzeitig erfüllt sein (AND-Verknüpfung)
        if (matches && !m_filterMessage.isEmpty()) {
            matches = source->messages().at(source_row).content().contains(m_filterMessage, Qt::CaseInsensitive);
        }

        if (matches && !m_filterNick.isEmpty()) {
            matches = source->messages().at(source_row).source().contains(m_filterNick, Qt::CaseInsensitive);
        }

        return matches;
    }
    
private:
    QString m_filterMessage;
    QString m_filterNick;
};

}

#endif
