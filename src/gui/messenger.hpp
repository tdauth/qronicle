#ifndef CHRONICLE_MESSENGER_HPP
#define CHRONICLE_MESSENGER_HPP

#include <QList>
#include <QString>
#include <QStringList>

#include "message.hpp"

namespace chronicle {
    
class Messenger {
    public:
        typedef QList<Message> Messages;
        
        virtual Messages loadFile(const QString &filePath) = 0;
        virtual Messages loadDirectory(const QString &dirPath) = 0;
        virtual QStringList defaultDirectories() = 0;
    
};

}

#endif
