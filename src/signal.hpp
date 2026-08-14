#ifndef QRONICLE_SIGNAL_HPP
#define QRONICLE_SIGNAL_HPP

#include "messenger.hpp"

namespace qronicle {

class Signal : public Messenger {
    public:
        virtual QString id() const override;
        virtual Messages loadFile(const QString &filePath) override;
        virtual Messages loadDirectories(const QStringList &dirPaths) override;
};

}

#endif
