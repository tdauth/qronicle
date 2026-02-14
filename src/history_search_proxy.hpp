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
    Q_PROPERTY(QString filterProtocol READ filterProtocol WRITE setFilterProtocol NOTIFY filterChanged)
    
public:
    Q_INVOKABLE int getUnfilteredIndex(int currentProxyRow);
    
    using QSortFilterProxyModel::QSortFilterProxyModel;
    
    // Getter & Setter
    QString filterMessage() const { return m_filterMessage; }
    void setFilterMessage(const QString &f) { 
        if (m_filterMessage == f) {
            return;
        }
        
        m_filterMessage = f; 
        
        if (f.isEmpty()) {
            // Der "Turbo" für das Leeren: Blockiert Einzel-Updates
            beginResetModel(); 
            setFilterFixedString(""); 
            endResetModel();
        } else {
            // Normales Filtern beim Tippen
            setFilterFixedString(f); 
        }
        
        emit filterChanged();
        
    }

    QString filterNick() const { return m_filterNick; }
    void setFilterNick(const QString &f) {
        m_filterNick = f; 
        
        if (f.isEmpty()) {
            // Der "Turbo" für das Leeren: Blockiert Einzel-Updates
            beginResetModel(); 
            setFilterFixedString(""); 
            endResetModel();
        } else {
            // Normales Filtern beim Tippen
            setFilterFixedString(f); 
        }
        
        emit filterChanged(); 
        
    }
    
    QString filterProtocol() const { return m_filterProtocol; }
    void setFilterProtocol(const QString &f) {
        m_filterProtocol = f;
        
        if (f.isEmpty()) {
            // Der "Turbo" für das Leeren: Blockiert Einzel-Updates
            beginResetModel(); 
            setFilterFixedString(""); 
            endResetModel();
        } else {
            // Normales Filtern beim Tippen
            setFilterFixedString(f); 
        }
        
        emit filterChanged();
        
    }

signals:
    void filterMessageChanged();
    void filterNickChanged();
    void filterProtocolChanged();
    
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
        
        if (matches && !m_filterProtocol.isEmpty()) {
            matches = source->messages().at(source_row).protocol().contains(m_filterProtocol, Qt::CaseInsensitive);
        }

        return matches;
    }
    
private:
    QString m_filterMessage;
    QString m_filterNick;
    QString m_filterProtocol;
};

}

#endif
