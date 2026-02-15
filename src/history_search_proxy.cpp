#include "history_search_proxy.hpp"

namespace qronicle {

int HistorySearchProxy::getUnfilteredIndex(int currentProxyRow){
    QModelIndex sourceIndex = this->mapToSource(this->index(currentProxyRow, 0));
    
    beginResetModel();
    m_filterFilePath.clear();
    m_filterMessage.clear();  // Internen String leeren
    m_filterNick.clear();
    m_filterTarget.clear();
    m_filterMessenger.clear();
    m_filterProtocol.clear();
    this->setFilterFixedString(""); 
    endResetModel();

    // WICHTIG: Signale feuern, damit QML die Änderung sieht!
    emit filterFilePathChanged();
    emit filterMessageChanged(); 
    emit filterNickChanged();
    emit filterTargetChanged();
    emit filterMessengerChanged();
    emit filterProtocolChanged();

    return sourceIndex.row();
}

bool HistorySearchProxy::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    auto *source = static_cast<HistoryModel*>(sourceModel());
    bool matches = true;
    
    // Logik: Alle gesetzten Filter müssen gleichzeitig erfüllt sein (AND-Verknüpfung)
    if (matches && !m_filterFilePath.isEmpty()) {
        matches = source->messages().at(source_row).filePath().contains(m_filterFilePath, Qt::CaseInsensitive);
    }
    
    if (matches && !m_filterMessage.isEmpty()) {
        matches = source->messages().at(source_row).content().contains(m_filterMessage, Qt::CaseInsensitive);
    }

    if (matches && !m_filterNick.isEmpty()) {
        matches = source->messages().at(source_row).source().contains(m_filterNick, Qt::CaseInsensitive);
    }
    
    if (matches && !m_filterTarget.isEmpty()) {
        matches = source->messages().at(source_row).destination().contains(m_filterTarget, Qt::CaseInsensitive);
    }
    
    if (matches && !m_filterMessenger.isEmpty()) {
        matches = source->messages().at(source_row).messenger().contains(m_filterMessenger, Qt::CaseInsensitive);
    }
    
    if (matches && !m_filterProtocol.isEmpty()) {
        matches = source->messages().at(source_row).protocol().contains(m_filterProtocol, Qt::CaseInsensitive);
    }

    return matches;
}

void HistorySearchProxy::triggerFilter(const QString &f) {
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

}
