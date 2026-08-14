#ifndef QRONICLE_KNUDDELS_HPP
#define QRONICLE_KNUDDELS_HPP

#include "messenger.hpp"

namespace qronicle {

class Knuddels : public Messenger {
    public:
        virtual QString id() const override;
        virtual Messages loadFile(const QString &filePath) override;
        virtual Messages loadDirectories(const QStringList &dirPaths) override;
        virtual QStringList defaultDirectories() override;
};

}

#endif
