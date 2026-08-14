#ifndef QRONICLE_WHATSAPP_HPP
#define QRONICLE_WHATSAPP_HPP

#include "messenger.hpp"

namespace qronicle {

class WhatsApp : public Messenger {
    public:
        virtual QString id() const override;
        virtual Messages loadFile(const QString &filePath) override;
        virtual Messages loadDirectories(const QStringList &dirPaths) override;
        virtual QStringList defaultDirectories() override;
};

}

#endif
