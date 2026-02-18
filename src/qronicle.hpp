#ifndef CHRONICLE_QRONICLE_HPP
#define CHRONICLE_QRONICLE_HPP

#include "messenger.hpp"

namespace qronicle {

class Qronicle : public Messenger {
    public:
        virtual QString id() const override;
        virtual Messages loadFile(const QString &filePath) override;
        virtual Messages loadDirectories(const QStringList &dirPaths) override;
        virtual QStringList defaultDirectories() override;
};

}

#endif
