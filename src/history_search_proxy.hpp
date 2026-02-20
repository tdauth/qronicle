#ifndef CHRONICLE_HISTORY_SEARCH_PROXY_HPP
#define CHRONICLE_HISTORY_SEARCH_PROXY_HPP

#include <QSortFilterProxyModel>

#include "history_model.hpp"

namespace qronicle {

// Faster searches in GUI without using regexs.
class HistorySearchProxy : public QSortFilterProxyModel {
    Q_OBJECT

    // Properties für QML
    Q_PROPERTY(QString filterFilePath READ filterFilePath WRITE setFilterFilePath NOTIFY filterChanged)
    Q_PROPERTY(QString filterMessage READ filterMessage WRITE setFilterMessage NOTIFY filterChanged)
    Q_PROPERTY(QString filterParticipant READ filterParticipant WRITE setFilterParticipant NOTIFY filterChanged)
    Q_PROPERTY(QString filterSender READ filterSender WRITE setFilterSender NOTIFY filterChanged)
    Q_PROPERTY(QString filterTarget READ filterTarget WRITE setFilterTarget NOTIFY filterChanged)
    Q_PROPERTY(QString filterMessenger READ filterMessenger WRITE setFilterMessenger NOTIFY filterChanged)
    Q_PROPERTY(QString filterProtocol READ filterProtocol WRITE setFilterProtocol NOTIFY filterChanged)
    Q_PROPERTY(QDateTime filterFrom READ filterFrom WRITE setFilterFrom NOTIFY filterChanged)
    Q_PROPERTY(QDateTime filterTo READ filterTo WRITE setFilterTo NOTIFY filterChanged)

    Q_PROPERTY(int totalCount READ totalCount NOTIFY filterChanged)
    Q_PROPERTY(int filteredCount READ filteredCount NOTIFY filterChanged)
    Q_PROPERTY(QString dateRange READ dateRange NOTIFY filterChanged)

public:
    Q_INVOKABLE void copyToClipboard(const QString &text);
    Q_INVOKABLE int findIndexById(QVariant targetId);
    Q_INVOKABLE int getUnfilteredIndex(int currentProxyRow);
    Q_INVOKABLE QStringList getAllMessengers();
    Q_INVOKABLE QStringList getAllProtocols();
    Q_INVOKABLE QStringList getAllNickNames();
    
    Q_INVOKABLE QDateTime getFrom();
    Q_INVOKABLE QDateTime getTo();

    using QSortFilterProxyModel::QSortFilterProxyModel;

    int totalCount() const;
    int filteredCount() const;

    QString dateRange() const;

    // Getter & Setter
    QString filterFilePath() const { return m_filterFilePath; }
    void setFilterFilePath(const QString &f) {
        if (m_filterFilePath == f) {
            return;
        }

        m_filterFilePath = f;

        triggerFilter();

    }

    QString filterMessage() const { return m_filterMessage; }
    void setFilterMessage(const QString &f) {
        if (m_filterMessage == f) {
            return;
        }

        m_filterMessage = f;

        triggerFilter();

    }
    
    QString filterParticipant() const { return m_filterParticipant; }
    void setFilterParticipant(const QString &f) {
        if (m_filterParticipant == f) {
            return;
        }

        m_filterParticipant = f;

        triggerFilter();

    }

    QString filterSender() const { return m_filterSender; }
    void setFilterSender(const QString &f) {
        if (m_filterSender == f) {
            return;
        }

        m_filterSender = f;

        triggerFilter();

    }

    QString filterTarget() const { return m_filterTarget; }
    void setFilterTarget(const QString &f) {
        if (m_filterTarget == f) {
            return;
        }

        m_filterTarget = f;

        triggerFilter();
    }

    QString filterMessenger() const { return m_filterMessenger; }
    void setFilterMessenger(const QString &f) {
        if (m_filterMessenger == f) {
            return;
        }

        m_filterMessenger = f;

        triggerFilter();
    }

    QString filterProtocol() const { return m_filterProtocol; }
    void setFilterProtocol(const QString &f) {
        if (m_filterProtocol == f) {
            return;
        }

        m_filterProtocol = f;

        triggerFilter();
    }
    
    QDateTime filterFrom() const { return m_filterFrom; }
    void setFilterFrom(const QDateTime &f) {
        if (m_filterFrom == f || !f.isValid()) {
            return;
        }

        m_filterFrom = f;

        triggerFilter();
    }

    QDateTime filterTo() const { return m_filterTo; }
    void setFilterTo(const QDateTime &f) {
        if (m_filterTo == f || !f.isValid()) {
            return;
        }
        
        qDebug() << "Filter to changed" << f;

        m_filterTo = f;

        triggerFilter();
    }

signals:
    void filterFilePathChanged();
    void filterMessageChanged();
    void filterParticipantChanged();
    void filterSenderChanged();
    void filterTargetChanged();
    void filterMessengerChanged();
    void filterProtocolChanged();
    void filterFromChanged();
    void filterToChanged();

    void filterChanged();

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    QString m_filterFilePath;
    QString m_filterMessage;
    QString m_filterParticipant;
    QString m_filterSender;
    QString m_filterTarget;
    QString m_filterMessenger;
    QString m_filterProtocol;
    QDateTime m_filterFrom;
    QDateTime m_filterTo;

    void triggerFilter();
};

}

#endif
