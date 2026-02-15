#ifndef CHRONICLE_KOPETE_HPP
#define CHRONICLE_KOPETE_HPP

#include "messenger.hpp"

namespace qronicle {
    
class Kopete : public Messenger {
    public:
        virtual QString id() const override;
        virtual Messages loadFile(const QString &filePath) override;
        virtual Messages loadDirectories(const QStringList &dirPaths) override;
        virtual QStringList defaultDirectories() override;
};

}

#endif
