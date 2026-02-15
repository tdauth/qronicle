#ifndef CHRONICLE_HISTORY_MODEL_HPP
#define CHRONICLE_HISTORY_MODEL_HPP

#include <QSqlTableModel>
#include <QSqlRecord>

#include "messenger.hpp"

namespace qronicle {

class HistoryModel : public QSqlTableModel {
    Q_OBJECT
    
public:
    enum Roles {
        FilePathRole = Qt::UserRole + 1,
        LineNumberRole,
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

    explicit HistoryModel(QSqlDatabase db);
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    void applyFilters(const QString &filePath, const QString &message, 
                               const QString &nick, const QString &target,
                               const QString &messenger, const QString &protocol);
};

}

#endif
