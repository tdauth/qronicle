#ifndef QRONICLE_AVATAR_PROVIDER_HPP
#define QRONICLE_AVATAR_PROVIDER_HPP

#include <QQuickImageProvider>
#include <QPixmapCache>

#include "messenger.hpp"

namespace qronicle {

class AvatarProvider : public QQuickImageProvider {

public:
    AvatarProvider(Messenger::Avatars &&avatars) : QQuickImageProvider(QQuickImageProvider::Image, QQuickImageProvider::ForceAsynchronousImageLoading), m_avatars(std::move(avatars)) {
        m_defaultAvatar = QImage(":/icons/user");
        if (m_defaultAvatar.isNull()) {
            qWarning() << "Failed to load default avatar resource!";
        }
    }

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override {
        QString cacheKey = id + "_" + QString::number(requestedSize.width()) + "x" + QString::number(requestedSize.height());
        QPixmap cachedPixmap;

        if (QPixmapCache::find(cacheKey, &cachedPixmap)) {
            if (size) *size = cachedPixmap.size();
            return cachedPixmap.toImage();
        }

        QImage img;
        {
            // Falls du Multithreading nutzt, hier den Read-Lock setzen
            // QReadLocker locker(&m_lock);
            img = m_avatars.value(id);
        }

        if (img.isNull()) {
            img = m_defaultAvatar;
        }

        if (requestedSize.isValid() && (img.width() != requestedSize.width() || img.height() != requestedSize.height())) {
            img = img.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        QPixmapCache::insert(cacheKey, QPixmap::fromImage(img));

        if (size) {
            *size = img.size();
        }

        return img;
    }

private:
        // Make sure this is never modified since asynchronous image loading will allow concurrent access.
        Messenger::Avatars m_avatars;
        QImage m_defaultAvatar;
};

}

#endif
