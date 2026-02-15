#include <QGuiApplication>
#include <QCommandLineParser>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QSortFilterProxyModel>
#include <QDir>
#include <QDirIterator>
#include <QStandardPaths>
#include <QtConcurrent>
#include <QFuture>
#include <QTranslator>
#include <QLibraryInfo>
#include <QLocale>

#include "database.hpp"
#include "history_model.hpp"
#include "history_search_proxy.hpp"
#include "avatar_provider.hpp"
#include "kopete.hpp"
#include "trillian.hpp"
#include "facebook.hpp"
#include "skype.hpp"
#include "whatsapp.hpp"
#include "psi.hpp"

using namespace qronicle;

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

QMap<QString, QString> loadCustomAliases() {
    QMap<QString, QString> aliases;
    
    QSettings settings;
    settings.beginGroup("Aliases");
    QStringList keys = settings.allKeys();
    
    for (const QString &key : keys) {
        aliases.insert(key, settings.value(key).toString());
    }
    
    settings.endGroup();
    
    qDebug() << "Loaded" << aliases.size() << "aliases from settings.";
    return aliases;
}

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    
    QTranslator translator;
    
    // 1. Die Systemsprache ermitteln (z.B. "de_DE")
    QLocale locale = QLocale::system(); 

    // 2. Die .qm Datei laden
    // "app" ist der Präfix aus deinem Dateinamen (app_de.ts -> "app")
    // ":/i18n" ist der Standardpfad, den CMake für Übersetzungen nutzt
    if (translator.load(locale, "app", "_", ":/i18n")) {
        app.installTranslator(&translator);
    }

    // Optional: Auch Standard-Texte von Qt selbst übersetzen (z.B. "Cancel", "Open")
    QTranslator qtTranslator;
    if (qtTranslator.load(locale, "qt", "_", 
        QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        app.installTranslator(&qtTranslator);
    }
    
    app.setApplicationName(QObject::tr("qronicle"));
    app.setApplicationVersion("1.0");
    app.setWindowIcon(QIcon(":/icons/qronicle")); 
    
    /*
    QDirIterator it(":/", QDirIterator::Subdirectories);
    qDebug() << "--- All resources: ---";
    while (it.hasNext()) {
        QString icon = it.next();
        if (!icon.contains("breez")) {
            qDebug() << icon;
        }
    }
    */
    
    // 2. Parser aufsetzen
    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("qronicle"));
    parser.addHelpOption();
    parser.addVersionOption();
    
    QCommandLineOption optionNoDistinct = QCommandLineOption("no-distinct", QObject::tr("Allows duplicated messages."));
    parser.addOption(optionNoDistinct);
    
    QList<std::shared_ptr<Messenger>> messengers;
    messengers << std::make_shared<Kopete>();
    messengers << std::make_shared<Trillian>();
    messengers << std::make_shared<Facebook>();
    messengers << std::make_shared<Skype>();
    messengers << std::make_shared<WhatsApp>();
    messengers << std::make_shared<Psi>();
    
    QMap<std::shared_ptr<Messenger>, QCommandLineOption*> optionMap;

    for (const auto& messenger : messengers) {
        auto* opt = new QCommandLineOption(messenger->id(), QObject::tr("%1 directories.").arg(messenger->id()), "path");
        parser.addOption(*opt);
        optionMap.insert(messenger, opt);
    }
    
    parser.process(app);
    
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
    Messenger::Messages allMessages;
    
    for (auto &future : futures) {
        allMessages.append(future.result());
    }
    
    qDebug() << "All messages" << allMessages.size();

    // Avatare einsammeln (nachdem die Threads fertig sind)
    Messenger::Avatars allAvatars;
    
    for (auto it = optionMap.begin(); it != optionMap.end(); ++it) {
        if (parser.isSet(*(it.value()))) {
            allAvatars.insert(it.key()->avatars());
        }
    }
    
    qDebug() << "All avatars" << allAvatars.size();
    
    allAvatars.insert(loadCustomAvatars());
    
    qDebug() << "All avatars with custom" << allAvatars.size();
    
    Database db;
    db.saveMessages(allMessages);
    // apply custom aliases before displaying anything
    db.applyAliases(loadCustomAliases());
    
    auto* baseModel = new HistoryModel(db.db());
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
