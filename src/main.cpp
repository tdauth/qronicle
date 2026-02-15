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
#include <QtConcurrent>
#include <QFuture>

#include "history_model.hpp"
#include "history_search_proxy.hpp"
#include "avatar_provider.hpp"
#include "kopete.hpp"
#include "trillian.hpp"
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
    
    QCommandLineOption optionDistinct = QCommandLineOption("distinct", QObject::tr("Ignore duplicated messages."));
    parser.addOption(optionDistinct);
    
    QList<std::shared_ptr<Messenger>> messengers;
    messengers << std::make_shared<Kopete>();
    messengers << std::make_shared<Trillian>();
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
    QList<Messenger::Messages> results;
    QList<QFuture<Messenger::Messages>> futures;
    const QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    qDebug() << "Config path" << configPath;
    
    for (auto it = optionMap.begin(); it != optionMap.end(); ++it) {
        auto messenger = it.key();
        auto* option = it.value();
        
        if (parser.isSet(*option)) {
            QStringList paths = parser.values(*option);
            QString appPath = QDir(configPath).filePath(messenger->id());
            
            paths << appPath;
            paths << messenger->defaultDirectories();
            
            qDebug() << "Queueing" << messenger->id() << "with" << paths.size() << "paths";

            // Wir starten das Laden pro Messenger in einem eigenen Thread
            futures << QtConcurrent::run([messenger, paths]() {
                return messenger->loadDirectories(paths);
            });
        }
    }

    // Ergebnisse einsammeln (Warten auf alle Messenger)
    for (auto &future : futures) {
        allMessages.append(future.result());
    }

    // Avatare einsammeln (nachdem die Threads fertig sind)
    for (auto it = optionMap.begin(); it != optionMap.end(); ++it) {
        if (parser.isSet(*(it.value()))) {
            allAvatars.insert(it.key()->avatars());
        }
    }
    
    qDebug() << "All messages" << allMessages.size();
    qDebug() << "All avatars" << allAvatars.size();
    
    QList<Message> distinctMessages;
    QSet<Message> seen;

    for (const Message &msg : allMessages) {
        if (parser.isSet(optionDistinct)) {
            if (!seen.contains(msg)) {
                seen.insert(msg);
                distinctMessages.append(msg);
            }
        } else {
            distinctMessages.append(msg);
        }
    }
    
    qDebug() << "Distinct messages" << distinctMessages.size();
    
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
    
    for (Message &msg : distinctMessages) {
        if (aliases.contains(msg.source())) {
            msg.setSource(aliases.value(msg.source()));
        }

        if (aliases.contains(msg.destination())) {
            msg.setDestination(aliases.value(msg.destination()));
        }
    }
    
    std::sort(distinctMessages.begin(), distinctMessages.end(), [](const Message &a, const Message &b) {
        return a.timestamp() < b.timestamp();
    });

    
    allAvatars.insert(loadCustomAvatars());
    
    qDebug() << "All avatars with custom" << allAvatars.size();
    
    auto* baseModel = new HistoryModel(std::move(distinctMessages));
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
