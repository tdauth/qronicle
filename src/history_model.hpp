#ifndef CHRONICLE_HISTORY_MODEL_HPP
#define CHRONICLE_HISTORY_MODEL_HPP

#include <QAbstractListModel>

#include "messenger.hpp"

namespace chronicle {

class HistoryModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        FilePathRole = Qt::UserRole + 1,
        ContentRole,
        TimestampRole,
        SenderRole,
        SenderNickRole,
        SenderAvatarRole,
        TargetRole,
        TargetNickRole,
        TargetAvatarRole,
        MessengerRole,
        ProtocolRole
    };

    explicit HistoryModel(Messenger::Messages &&msgs, QObject* parent = nullptr)
        : QAbstractListModel(parent), m_messages(std::move(msgs)) {}

    int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        return static_cast<int>(m_messages.size());
    }

    QVariant data(const QModelIndex& index, int role) const override {
        if (!index.isValid() || index.row() >= m_messages.size()) return {};
        const auto& msg = m_messages[index.row()];

        switch (role) {
            case FilePathRole: return msg.filePath();
            case ContentRole: return msg.contentHtml();
            case SenderRole:    return msg.source();
            case SenderNickRole:    return msg.sourceNick();
            case SenderAvatarRole: return msg.source();
            case TargetRole:    return msg.destination();
            case TargetNickRole:    return msg.destinationNick();
            case TargetAvatarRole: return msg.destination();
            case MessengerRole:  return msg.messenger();
            case ProtocolRole:  return msg.protocol();
            case TimestampRole: return msg.timestamp();
            default: return {};
        }
    }

    QHash<int, QByteArray> roleNames() const override {
        return {
            {FilePathRole, "filePath"},
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
    
    Messenger::Messages messages() const {
        return m_messages;
    }

private:
    Messenger::Messages m_messages;
};

}

#endif
