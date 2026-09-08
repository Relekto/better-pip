#include <QApplication>
#include "AppController.h"
#include "SingleInstance.h"
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QTimer>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("Better PiP"));
    QCoreApplication::setOrganizationName(QStringLiteral("BetterPiP"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1.0"));
    QQuickStyle::setStyle(QStringLiteral("Basic"));
    QApplication::setQuitOnLastWindowClosed(false);
    const bool smokeTest = app.arguments().contains(QStringLiteral("--smoke-test"));
    pip::SingleInstance instance;
    if (!smokeTest && !instance.start()) return 0;
    pip::AppController controller(smokeTest);
    QObject::connect(&instance, &pip::SingleInstance::activationRequested,
                     &controller, &pip::AppController::showControls);
    QQmlApplicationEngine engine;
    engine.setInitialProperties({{QStringLiteral("controller"), QVariant::fromValue(&controller)}});
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app,
                     [] { QCoreApplication::exit(1); }, Qt::QueuedConnection);
    engine.loadFromModule(QStringLiteral("BetterPiP"), QStringLiteral("Main"));
    if (app.arguments().contains(QStringLiteral("--smoke-test"))) {
        QTimer::singleShot(500, &app, &QCoreApplication::quit);
    }
    return app.exec();
}
