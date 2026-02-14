#ifndef CHRONICLE_WHATSAPP_HPP
#define CHRONICLE_WHATSAPP_HPP

#include "messenger.hpp"

namespace chronicle {
    
class WhatsApp : public Messenger {
    public:
        virtual QString id() const override;
        virtual Messages loadFile(const QString &filePath) override;
        virtual Messages loadDirectory(const QString &dirPath) override;
        virtual QStringList defaultDirectories() override;
};

}

#endif
