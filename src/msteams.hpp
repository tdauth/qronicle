#ifndef QRONICLE_MSTEAMS_HPP
#define QRONICLE_MSTEAMS_HPP

#include "messenger.hpp"

namespace qronicle {

class MsTeams : public Messenger {
    public:
        virtual QString id() const override;
        virtual Messages loadFile(const QString &filePath) override;
        virtual Messages loadDirectories(const QStringList &dirPaths) override;
};

}

#endif
