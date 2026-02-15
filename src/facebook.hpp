#ifndef CHRONICLE_FACEBOOK_HPP
#define CHRONICLE_FACEBOOK_HPP

#include "messenger.hpp"

namespace qronicle {

class Facebook : public Messenger {
    public:
        virtual QString id() const override;
        virtual Messages loadFile(const QString &filePath) override;
        virtual Messages loadDirectories(const QStringList &dirPaths) override;
        virtual QStringList defaultDirectories() override;
};

}

#endif
