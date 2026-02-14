#ifndef CHRONICLE_KOPETE_HPP
#define CHRONICLE_KOPETE_HPP

#include "messenger.hpp"

namespace chronicle {
    
class Kopete : public Messenger {
    public:
        virtual Messages loadFile(const QString &filePath) override;
        virtual Messages loadDirectory(const QString &dirPath) override;
        virtual QStringList defaultDirectories() override;
};

}

#endif
