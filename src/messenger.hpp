#ifndef CHRONICLE_MESSENGER_HPP
#define CHRONICLE_MESSENGER_HPP

#include <QList>
#include <QString>
#include <QStringList>
#include <QHash>
#include <QImage>

#include "message.hpp"

namespace chronicle {
    
class Messenger {
    public:
        typedef QList<Message> Messages;
        typedef QHash<QString, QImage> Avatars;
        
        virtual QString id() const = 0;
        virtual Messages loadFile(const QString &filePath) = 0;
        virtual Messages loadDirectory(const QString &dirPath) = 0;
        virtual QStringList defaultDirectories() = 0;
        
        Avatars avatars() const {
            return m_avatars;
        }
        
    protected:
        Avatars m_avatars;
    
};

}

#endif
