#ifndef CHRONICLE_HISTORY_MODEL_HPP
#define CHRONICLE_HISTORY_MODEL_HPP

#include <QAbstractListModel>

#include "messenger.hpp"

namespace chronicle {

class HistoryModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        ContentRole = Qt::UserRole + 1,
        TimestampRole,
        SenderRole,
        SenderNickRole,
        SenderAvatarRole,
        TargetRole,
        TargetNickRole,
        TargetAvatarRole,
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
            case ContentRole: return msg.contentHtml();
            case SenderRole:    return msg.source();
            case SenderNickRole:    return msg.sourceNick();
            case SenderAvatarRole: return msg.source();
            case TargetRole:    return msg.destination();
            case TargetNickRole:    return msg.destinationNick();
            case TargetAvatarRole: return msg.destination();
            case ProtocolRole:  return msg.protocol();
            case TimestampRole: return msg.timestamp();
            default: return {};
        }
    }

    QHash<int, QByteArray> roleNames() const override {
        return {
            {ContentRole, "messageText"},
            {TimestampRole, "time"},
            {SenderRole, "sourceId"},
            {SenderNickRole, "sourceNick"},
            {SenderAvatarRole, "sourceAvatar"},
            {TargetRole, "targetId"},
            {TargetNickRole, "targetNick"},
            {TargetAvatarRole, "targetAvatar"},
            {ProtocolRole, "protocol"}
        };
    }

private:
    Messenger::Messages m_messages;
};

}

#endif
