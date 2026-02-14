#include <QGuiApplication>
#include <QCommandLineParser>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QSortFilterProxyModel>
#include <QDir>
#include <QDirIterator>
#include <QStandardPaths>
#include <QSettings>

#include "history_model.hpp"
#include "history_search_proxy.hpp"
#include "avatar_provider.hpp"
#include "kopete.hpp"
#include "facebook.hpp"
#include "skype.hpp"
#include "whatsapp.hpp"

using namespace chronicle;

Messenger::Avatars loadCustomAvatars() {
    Messenger::Avatars avatarCache;

    // 1. Pfad ermitteln (z.B. ~/.config/Chronicle/avatars)
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QString avatarPath = QDir(configPath).filePath("avatars");
    
    qDebug() << "Loading custom avatars from" << avatarPath;

    // 2. Iterator für Bilder (png, jpg, jpeg)
    QDirIterator it(avatarPath, {"*.png", "*.jpg", "*.jpeg"}, QDir::Files);

    while (it.hasNext()) {
        QString filePath = it.next();
        QFileInfo fileInfo(filePath);

        // Dateiname ohne Endung (z.B. "max_mustermann")
        QString key = fileInfo.baseName();

        QImage img(filePath);
        if (!img.isNull()) {
            avatarCache.insert(key, img);
        }
    }
    
    qDebug() << "Custom avatars" << avatarCache.size();

    return avatarCache;
}

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    app.setApplicationName(QObject::tr("Chronicle"));
    app.setApplicationVersion("1.0");
    app.setWindowIcon(QIcon(":/icons/chronicle")); 
    
    // Teste den Haupt-Präfix
    QDirIterator it(":/icons", QDirIterator::Subdirectories);
    qDebug() << "--- Alle geladenen Icons: ---";
    while (it.hasNext()) {
        QString icon = it.next();
        if (!icon.contains("breez")) {
            qDebug() << icon;
        }
    }
    
    // 2. Parser aufsetzen
    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("Chronicle"));
    parser.addHelpOption();
    parser.addVersionOption();
    
    QList<std::shared_ptr<Messenger>> messengers;
    messengers << std::make_shared<Kopete>();
    messengers << std::make_shared<Facebook>();
    messengers << std::make_shared<Skype>();
    messengers << std::make_shared<WhatsApp>();
    
    QMap<std::shared_ptr<Messenger>, QCommandLineOption*> optionMap;

    for (const auto& messenger : messengers) {
        auto* opt = new QCommandLineOption(messenger->id(), QObject::tr("%1 directories.").arg(messenger->id()), "path");
        parser.addOption(*opt);
        optionMap.insert(messenger, opt);
    }
    
    parser.process(app);
    
    Messenger::Messages allMessages;
    Messenger::Avatars allAvatars;
    
    for (auto it = optionMap.begin(); it != optionMap.end(); ++it) {
        auto messenger = it.key();
        auto* option = it.value();
        
        if (parser.isSet(*option)) {
            QStringList paths = parser.values(*option);
            QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
            QString appPath = QDir(configPath).filePath(messenger->id());
            
            paths << appPath;
            paths << messenger->defaultDirectories();
            
            for (const QString &path : paths) {
                qDebug() << "Loading" << messenger->id() << "from" << path;
                allMessages.append(messenger->loadDirectory(path));
                allAvatars.insert(messenger->avatars());
            }
        }
    }
    
    std::sort(allMessages.begin(), allMessages.end(), [](const Message &a, const Message &b) {
        return a.timestamp() < b.timestamp();
    });
    
    QSettings settings("Chronicle", "Chronicle");
    settings.beginGroup("Aliases");
    QMap<QString, QString> aliases;
    // childKeys() liefert alle Namen/Nummern (die "Keys" in der INI)
    QStringList keys = settings.childKeys(); 

    for (const QString &key : keys) {
        aliases.insert(key, settings.value(key).toString());
    }
    settings.endGroup();
    
    qDebug() << "Aliases" << aliases;
    
    for (Message &msg : allMessages) {
        if (aliases.contains(msg.source())) {
            msg.setSource(aliases.value(msg.source()));
        }

        if (aliases.contains(msg.destination())) {
            msg.setDestination(aliases.value(msg.destination()));
        }
    }
    
    qDebug() << "All messages" << allMessages.size();
    qDebug() << "All avatars" << allAvatars.size();
    
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    qDebug() << "Config path" << configPath;

    
    allAvatars.insert(loadCustomAvatars());
    
    qDebug() << "All avatars with custom" << allAvatars.size();
    
    auto* baseModel = new HistoryModel(std::move(allMessages));
    auto *proxyModel = new HistorySearchProxy(&app);
    proxyModel->setSourceModel(baseModel);
    proxyModel->setFilterRole(-1); // Search for all.
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setRecursiveFilteringEnabled(false);
    proxyModel->setDynamicSortFilter(false);

    QQmlApplicationEngine engine;
    engine.addImageProvider(QLatin1String("avatars"), new AvatarProvider(std::move(allAvatars)));
    engine.rootContext()->setContextProperty("chatModel", proxyModel);
    engine.loadFromModule("Chronicle", "Main");
    
    return app.exec();
}
