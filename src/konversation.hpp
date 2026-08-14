#ifndef QRONICLE_KONVERSATION_HPP
#define QRONICLE_KONVERSATION_HPP

#include "messenger.hpp"

namespace qronicle {

class Konversation : public Messenger {
    public:
        virtual QString id() const override;
        virtual Messages loadFile(const QString &filePath) override;
        virtual Messages loadDirectories(const QStringList &dirPaths) override;
        virtual QStringList defaultDirectories() override;
};

}

#endif
