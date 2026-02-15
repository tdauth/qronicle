#ifndef CHRONICLE_HISTORY_SEARCH_PROXY_HPP
#define CHRONICLE_HISTORY_SEARCH_PROXY_HPP

#include <QSortFilterProxyModel>

#include "history_model.hpp"

namespace chronicle {

// Faster searches in GUI without using regexs.
class HistorySearchProxy : public QSortFilterProxyModel {
    Q_OBJECT
    
    // Properties für QML
    Q_PROPERTY(QString filterFilePath READ filterFilePath WRITE setFilterFilePath NOTIFY filterChanged)
    Q_PROPERTY(QString filterMessage READ filterMessage WRITE setFilterMessage NOTIFY filterChanged)
    Q_PROPERTY(QString filterNick READ filterNick WRITE setFilterNick NOTIFY filterChanged)
    Q_PROPERTY(QString filterTarget READ filterTarget WRITE setFilterTarget NOTIFY filterChanged)
    
    Q_PROPERTY(QString filterMessenger READ filterMessenger WRITE setFilterMessenger NOTIFY filterChanged)
    Q_PROPERTY(QString filterProtocol READ filterProtocol WRITE setFilterProtocol NOTIFY filterChanged)
    
public:
    Q_INVOKABLE int getUnfilteredIndex(int currentProxyRow);
    
    using QSortFilterProxyModel::QSortFilterProxyModel;
    
    // Getter & Setter
    QString filterFilePath() const { return m_filterFilePath; }
    void setFilterFilePath(const QString &f) { 
        if (m_filterFilePath == f) {
            return;
        }
        
        m_filterFilePath = f; 
        
        triggerFilter(f);
        
    }
    
    QString filterMessage() const { return m_filterMessage; }
    void setFilterMessage(const QString &f) { 
        if (m_filterMessage == f) {
            return;
        }
        
        m_filterMessage = f; 
        
        triggerFilter(f);
        
    }

    QString filterNick() const { return m_filterNick; }
    void setFilterNick(const QString &f) {
        if (m_filterNick == f) {
            return;
        }
        
        m_filterNick = f; 
        
        triggerFilter(f);
        
    }
    
    QString filterTarget() const { return m_filterTarget; }
    void setFilterTarget(const QString &f) {
        if (m_filterTarget == f) {
            return;
        }
        
        m_filterTarget = f; 
        
        triggerFilter(f);
        
    }
    
    
    QString filterMessenger() const { return m_filterMessenger; }
    void setFilterMessenger(const QString &f) {
        if (m_filterMessenger == f) {
            return;
        }
        
        m_filterMessenger = f;
        
        triggerFilter(f);
        
    }
    
    QString filterProtocol() const { return m_filterProtocol; }
    void setFilterProtocol(const QString &f) {
        if (m_filterProtocol == f) {
            return;
        }
        
        m_filterProtocol = f;
        
        triggerFilter(f);
        
    }

signals:
    void filterFilePathChanged();
    void filterMessageChanged();
    void filterNickChanged();
    void filterTargetChanged();
    void filterMessengerChanged();
    void filterProtocolChanged();
    
    void filterChanged();
    
protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    
private:
    QString m_filterFilePath;
    QString m_filterMessage;
    QString m_filterNick;
    QString m_filterTarget;
    QString m_filterMessenger;
    QString m_filterProtocol;
    
    void triggerFilter(const QString &f);
};

}

#endif
