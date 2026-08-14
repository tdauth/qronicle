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
#include "qronicle.hpp"
#include "kopete.hpp"
#include "konversation.hpp"
#include "trillian.hpp"
#include "signal.hpp"
#include "facebook.hpp"
#include "instagram.hpp"
#include "telegram.hpp"
#include "skype.hpp"
#include "whatsapp.hpp"
#include "psi.hpp"
#include "amsn.hpp"
#include "knuddels.hpp"

using namespace qronicle;

// ~/.config/qronicle/
const QString CONFIG_PATH = QDir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)).filePath("qronicle");

Messenger::Avatars loadCustomAvatars() {
    Messenger::Avatars avatarCache;

    // 1. Pfad ermitteln (z.B. ~/.config/qronicle/avatars)
    const QString avatarPath = QDir(CONFIG_PATH).filePath("avatars");

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

    qDebug() << "QSettings format:" << settings.format();
    qDebug() << "QSettings organization:" << settings.organizationName();
    qDebug() << "QSettings application:" << settings.applicationName();
    qDebug() << "QSettings fileName:" << settings.fileName();

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
    QCoreApplication::setOrganizationName("qronicle"); // important for settings folder ~/.config/qronicle
    QCoreApplication::setApplicationName("qronicle");
    QGuiApplication app(argc, argv);

    QTranslator translator;
    QLocale locale = QLocale::system();
    if (translator.load(locale, "app", "_", ":/i18n")) {
        app.installTranslator(&translator);
    }

    QTranslator qtTranslator;
    if (qtTranslator.load(locale, "qt", "_",
        QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        app.installTranslator(&qtTranslator);
    }

    app.setOrganizationName("qronicle"); // important for settings folder ~/.config/qronicle
    app.setApplicationName(QObject::tr("qronicle"));
    app.setApplicationVersion("1.0");
    app.setWindowIcon(QIcon(":/icons/qronicle"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("qronicle"));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption optionClear = QCommandLineOption("clear", QObject::tr("Clears the existing database before loading messages."));
    parser.addOption(optionClear);

    QList<std::shared_ptr<Messenger>> messengers;
    messengers << std::make_shared<Qronicle>();
    messengers << std::make_shared<Kopete>();
    messengers << std::make_shared<Konversation>();
    messengers << std::make_shared<Trillian>();
    messengers << std::make_shared<Signal>();
    messengers << std::make_shared<Facebook>();
    messengers << std::make_shared<Instagram>();
    messengers << std::make_shared<Telegram>();
    messengers << std::make_shared<Skype>();
    messengers << std::make_shared<WhatsApp>();
    messengers << std::make_shared<Psi>();
    messengers << std::make_shared<Amsn>();
    messengers << std::make_shared<Knuddels>();

    QMap<std::shared_ptr<Messenger>, QCommandLineOption*> optionMap;

    for (const auto& messenger : messengers) {
        auto* opt = new QCommandLineOption(messenger->id(), QObject::tr("%1 directories.").arg(messenger->id()), "path");
        parser.addOption(*opt);
        optionMap.insert(messenger, opt);
    }

    parser.process(app);

    QList<QFuture<Messenger::Messages>> futures;
    qDebug() << "Config path" << CONFIG_PATH;

    for (auto it = optionMap.begin(); it != optionMap.end(); ++it) {
        auto messenger = it.key();
        auto* option = it.value();

        QStringList paths;

        // ~/.config/qronicle/qronicle/, ~/.config/qronicle/kopete/ by default
        const QString appPath = QDir(CONFIG_PATH).filePath(messenger->id());

        if (QDir(appPath).exists() && QDir(appPath).isReadable()) {
            paths << appPath;
        }

        const QStringList defaultDirs = messenger->defaultDirectories();

        for (const QString &dir : defaultDirs) {
            if (QDir(dir).exists() && QDir(appPath).isReadable()) {
                paths << dir;
            }
        }

        if (parser.isSet(*option)) {
            paths << parser.values(*option);
        }

        qDebug() << "Queueing" << messenger->id() << "with" << paths.size() << "paths:" << paths;

        futures << QtConcurrent::run([messenger, paths]() {
            return messenger->loadDirectories(paths);
        });
    }

    Messenger::Messages allMessages;

    for (auto &future : futures) {
        allMessages.append(future.result());
    }

    qDebug() << "All messages" << allMessages.size();

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

    if (parser.isSet(optionClear)) {
        db.removeDatabaseFile();
    }

    db.saveMessages(allMessages);
    qDebug() << "Before applying alliases.";
    // apply custom aliases before displaying anything
    db.applyAliases(loadCustomAliases());
    qDebug() << "After applying alliases.";

    auto* baseModel = new HistoryModel(db.db());
    auto *proxyModel = new HistorySearchProxy(&app);
    proxyModel->setSourceModel(baseModel);
    proxyModel->setFilterRole(-1); // Search for all.
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setRecursiveFilteringEnabled(false);
    proxyModel->setDynamicSortFilter(false);
    proxyModel->setFilterFrom(baseModel->getFrom());
    proxyModel->setFilterTo(baseModel->getTo());

    qDebug() << "--- Registered files QRC ---";
    QDirIterator it(":", QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString path = it.next();
        if (path.endsWith(".qml") && !path.contains("breez")) {
            qDebug() << path;
        }
    }

    QQmlApplicationEngine engine;
    engine.addImageProvider(QLatin1String("avatars"), new AvatarProvider(std::move(allAvatars)));
    engine.rootContext()->setContextProperty("chatModel", proxyModel);
    engine.rootContext()->setContextProperty("appConfigPath", CONFIG_PATH);

    QLocale compilerLocale(QLocale::English);
    QDate buildDate = compilerLocale.toDate(QString(__DATE__).simplified(), "MMM d yyyy");
    QTime buildTime = QTime::fromString(__TIME__, "HH:mm:ss");
    QDateTime buildDateTime(buildDate, buildTime);
    engine.rootContext()->setContextProperty("buildDateTime", buildDateTime);

    engine.load(QUrl(QStringLiteral("qrc:/qronicle_qml/src/CustomSearchField.qml")));
    engine.load(QUrl(QStringLiteral("qrc:/qronicle_qml/src/DateTimePicker.qml")));
    engine.load(QUrl(QStringLiteral("qrc:/qronicle_qml/src/Main.qml")));

    return app.exec();
}
