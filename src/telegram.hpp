#ifndef QRONICLE_TELEGRAM_HPP
#define QRONICLE_TELEGRAM_HPP

#include "messenger.hpp"

namespace qronicle {

class Telegram : public Messenger {
    public:
        virtual QString id() const override;
        virtual Messages loadFile(const QString &filePath) override;
        virtual Messages loadDirectories(const QStringList &dirPaths) override;
};

}

#endif
