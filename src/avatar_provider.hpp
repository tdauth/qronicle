#ifndef CHRONICLE_AVATAR_PROVIDER_HPP
#define CHRONICLE_AVATAR_PROVIDER_HPP

#include <QQuickImageProvider>

#include "messenger.hpp"

namespace chronicle {
    
class AvatarProvider : public QQuickImageProvider {
public:
    AvatarProvider(Messenger::Avatars &&avatars) 
        : QQuickImageProvider(QQuickImageProvider::Image), m_avatars(std::move(avatars)) {}

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override {
        static const QImage defaultAvatar(":/icons/user");
        
        if (m_avatars.contains(id)) {
            //qDebug() << "Found in cache" << id;
            // id ist der Teil nach "image://avatars/"
            QImage img = m_avatars.value(id);
            if (!img.isNull()) {
                if (size) *size = img.size();
                return img;
            } else {
                //qWarning() << "Found avatar ID but QImage is NULL:" << id;
            }
        }
        // 2. Falls nicht gefunden oder Bild leer: Gib das Standard-Bild zurück
        if (size) *size = defaultAvatar.size();
        return defaultAvatar;
    }

private:
        Messenger::Avatars m_avatars;
};

}

#endif
