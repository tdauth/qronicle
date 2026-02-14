#include <QGuiApplication>
#include <QCommandLineParser>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSortFilterProxyModel>
#include <QDir>

#include "history_model.hpp"
#include "kopete.hpp"
#include "facebook.hpp"

using namespace chronicle;

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    app.setApplicationName(QObject::tr("Chronicle"));
    app.setApplicationVersion("1.0");
    
    // 2. Parser aufsetzen
    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("Chronicle"));
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption kopeteOption("kopete", QObject::tr("Directories with Kopete logs."), QObject::tr("directory path"));
    parser.addOption(kopeteOption);
    QCommandLineOption facebookOption("facebook", QObject::tr("Directories with messages directories from Facebook Messenger."), QObject::tr("directory path"));
    parser.addOption(facebookOption);
    parser.process(app);
    
    Messenger::Messages allMessages;
    
    Kopete kopete;
    QStringList kopetePaths = parser.values(kopeteOption);
    kopetePaths << kopete.defaultDirectories();
    
    for (auto p : kopetePaths) {
        allMessages.append(kopete.loadDirectory(p));
    }
    
    Facebook facebook;
    QStringList facebookPaths = parser.values(facebookOption);
    facebookPaths << facebook.defaultDirectories();
    
    for (auto p : facebookPaths) {
        allMessages.append(facebook.loadDirectory(p));
    }
    
    std::sort(allMessages.begin(), allMessages.end(), [](const Message &a, const Message &b) {
        return a.timestamp() < b.timestamp();
    });
    
    qDebug() << "All messages" << allMessages.size();
    
    auto* baseModel = new ChatMessageModel(std::move(allMessages));
    
    QSet<QString> uniqueSources;

    for (const Message &msg : allMessages) {
        uniqueSources.insert(msg.source());
    }

    QStringList sortedSources = uniqueSources.values();
    sortedSources.sort(Qt::CaseInsensitive);
    
    auto* proxyModel = new QSortFilterProxyModel(&app);
    proxyModel->setSourceModel(baseModel);
    proxyModel->setFilterRole(-1); // Search for all.
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setDynamicSortFilter(false);

    QQmlApplicationEngine engine;
    // Daten an QML übergeben
    engine.rootContext()->setContextProperty("chatPartners", QVariant::fromValue(sortedSources));
    engine.rootContext()->setContextProperty("chatModel", proxyModel);
    engine.loadFromModule("Chronicle", "Main");
    
    return app.exec();
}
