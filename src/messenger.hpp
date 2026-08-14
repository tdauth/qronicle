#ifndef QRONICLE_MESSENGER_HPP
#define QRONICLE_MESSENGER_HPP

#include <QList>
#include <QString>
#include <QStringList>
#include <QHash>
#include <QImage>

#include "message.hpp"

namespace qronicle {

class Messenger {
    public:
        typedef QList<Message> Messages;
        typedef QHash<QString, QImage> Avatars;

        virtual QString id() const = 0;
        virtual Messages loadFile(const QString &filePath) = 0;
        virtual Messages loadDirectories(const QStringList &dirPaths) = 0;
        virtual QStringList defaultDirectories() {
            return {};
        }

        Avatars avatars() const {
            return m_avatars;
        }

    protected:
        Avatars m_avatars;

        /**
         * Replaces all URLs with HTML a tags.
         */
        QString formatHtml(const QString &msg);

};

}

#endif
