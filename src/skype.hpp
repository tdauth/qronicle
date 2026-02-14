#ifndef CHRONICLE_SKYPE_HPP
#define CHRONICLE_SKYPE_HPP

#include "messenger.hpp"

namespace chronicle {
    
class Skype : public Messenger {
    public:
        virtual QString id() const override;
        virtual Messages loadFile(const QString &filePath) override;
        virtual Messages loadDirectory(const QString &dirPath) override;
        virtual QStringList defaultDirectories() override;
};

}

#endif
