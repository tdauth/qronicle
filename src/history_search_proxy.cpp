#include "history_search_proxy.hpp"

namespace chronicle {

int HistorySearchProxy::getUnfilteredIndex(int currentProxyRow){
    QModelIndex sourceIndex = this->mapToSource(this->index(currentProxyRow, 0));
    
    beginResetModel();
    m_filterMessage.clear();  // Internen String leeren
    m_filterNick.clear();
    m_filterProtocol.clear();
    this->setFilterFixedString(""); 
    endResetModel();

    // WICHTIG: Signale feuern, damit QML die Änderung sieht!
    emit filterMessageChanged(); 
    emit filterNickChanged();
    emit filterProtocolChanged();

    return sourceIndex.row();
}

}
