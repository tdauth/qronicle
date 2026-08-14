#ifndef QRONICLE_AMSN_HPP
#define QRONICLE_AMSN_HPP

#include <QHash>

#include "messenger.hpp"

namespace qronicle {

class Amsn : public Messenger {
    public:
        virtual QString id() const override;
        virtual Messages loadFile(const QString &filePath) override;
        virtual Messages loadDirectories(const QStringList &dirPaths) override;
        virtual QStringList defaultDirectories() override;

    private:
        struct Contact {
            QString cid;
            QString name; // email
            QString nick; // shown in .log files
            QString displaypicfile; // file name without extension of avatar
            QString mfn; // nick name for myself
            QString login; // email for myself
        };

        // TODO Store different address books and load the corresponding one for each log file to get the correct data.
        typedef QHash<QString, Contact> Contacts;
        Contacts m_contacts;

        void loadABooks(const QStringList &dirPaths);
        void loadAvatars(const QStringList &dirPaths);
};

}

#endif
