#ifndef CHRONICLE_TRILLIAN_HPP
#define CHRONICLE_TRILLIAN_HPP

#include "messenger.hpp"

namespace chronicle {
    
class Trillian : public Messenger {
    public:
        virtual QString id() const override;
        virtual Messages loadFile(const QString &filePath) override;
        virtual Messages loadDirectories(const QStringList &dirPaths) override;
        virtual QStringList defaultDirectories() override;
};

}

#endif
