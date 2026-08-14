#ifndef QRONICLE_PSI_HPP
#define QRONICLE_PSI_HPP

#include <optional>

#include "messenger.hpp"

namespace qronicle {

class Psi : public Messenger {
    public:
        virtual QString id() const override;
        virtual Messages loadFile(const QString &filePath) override;
        virtual Messages loadDirectories(const QStringList &dirPaths) override;
        virtual QStringList defaultDirectories() override;

    private:
        struct Account {
            QString name;
            QString jid;
            QString host;
            QString vCardFilePath;  // there is a vCard file for the jid in vcard, it can be tamino@cdauth.de and the vcard name could be tamino_at_cdauth.de.xml
        };

        struct CV {
            QString fn; // formatted name
            QString nickName;
            QString birthDate;
            QString email;
            QString title;
            QString role;
            QString description;
            QString url;
        };

        // jid as key
        typedef QMap<QString, Account> Accounts;
        // file path as key
        typedef QMap<QString, CV> Cvs;

        Accounts m_accounts;
        Cvs m_cvs;

        void readVCardFile(const QString &filePath);
        void readAccountsFile(const QString &filePath);

        std::optional<CV> matchingCv(const QString &filePath);
        std::optional<Account> matchingAccount(const QString &filePath);
};

}

#endif
