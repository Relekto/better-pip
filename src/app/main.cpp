#include "AppController.h"
#include "SingleInstance.h"
#include <QApplication>
#include <QIcon>
#include <QMessageBox>
#include <QQmlApplicationEngine>
#include <QQmlError>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QTimer>
#include <cstdio>

int main(int argc, char *argv[]) {
    if (qEnvironmentVariableIsSet("BETTER_PIP_DIAGNOSTICS")) {
        qInstallMessageHandler([](QtMsgType, const QMessageLogContext &, const QString &message) {
            const auto text = message.toLocal8Bit();
            std::fprintf(stderr, "%s\n", text.constData());
            std::fflush(stderr);
        });
    }
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("Better PiP"));
    QCoreApplication::setOrganizationName(QStringLiteral("BetterPiP"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1.0"));
    QGuiApplication::setWindowIcon(QIcon(QStringLiteral(":/assets/icon.svg")));
    QGuiApplication::setDesktopFileName(QStringLiteral("io.github.relekto.better-pip"));
    QQuickStyle::setStyle(QStringLiteral("Basic"));
    QApplication::setQuitOnLastWindowClosed(false);
    const bool smokeTest = app.arguments().contains(QStringLiteral("--smoke-test"));
    pip::SingleInstance instance;
    if (!smokeTest) {
        const auto result = instance.start();
        if (result == pip::SingleInstance::StartResult::Forwarded) {
            return 0;
        }
        if (result == pip::SingleInstance::StartResult::Failed) {
            QMessageBox::critical(nullptr, QStringLiteral("Better PiP"),
                                  QStringLiteral("The recovery control channel could not start: ") +
                                      instance.error());
            return 1;
        }
    }
    pip::AppController controller(smokeTest);
    QObject::connect(&instance, &pip::SingleInstance::activationRequested, &controller,
                     &pip::AppController::showControls);
    bool qmlFailed = false;
    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::warnings, &app,
                     [&qmlFailed](const QList<QQmlError> &errors) {
                         if (!errors.isEmpty()) {
                             qmlFailed = true;
                         }
                     });
    engine.setInitialProperties({{QStringLiteral("controller"), QVariant::fromValue(&controller)}});
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        [] { QCoreApplication::exit(1); }, Qt::QueuedConnection);
    engine.loadFromModule(QStringLiteral("BetterPiP"), QStringLiteral("Main"));
    const auto sourceArgument = app.arguments().indexOf(QStringLiteral("--smoke-source"));
    QTimer smokeDeadline;
    smokeDeadline.setSingleShot(true);
    QTimer sourceStart;
    sourceStart.setSingleShot(true);
    if (smokeTest && sourceArgument >= 0) {
        const auto sourceTitle = app.arguments().value(sourceArgument + 1);
        QObject::connect(&smokeDeadline, &QTimer::timeout, &app, [] { QCoreApplication::exit(1); });
        smokeDeadline.start(15000);
        for (auto *window : QGuiApplication::topLevelWindows()) {
            auto *quickWindow = qobject_cast<QQuickWindow *>(window);
            if (!quickWindow || window->title() == QStringLiteral("Better PiP")) {
                continue;
            }
            QObject::connect(
                quickWindow, &QQuickWindow::frameSwapped, &controller,
                [&controller, count = 0]() mutable {
                    if (!controller.hasFrame()) {
                        return;
                    }
                    ++count;
                    if (count == 3) {
                        controller.toggleLock();
                    }
                    if (count == 6) {
                        controller.unlock();
                    }
                    if (count == 9) {
                        QCoreApplication::quit();
                    }
                },
                Qt::QueuedConnection);
        }
        QObject::connect(&sourceStart, &QTimer::timeout, &controller, [&controller, sourceTitle] {
            controller.sources()->refresh();
            for (int row = 0; row < controller.sources()->rowCount(); ++row) {
                const auto index = controller.sources()->index(row, 0);
                if (index.data(pip::WindowSources::TitleRole).toString() == sourceTitle) {
                    controller.selectSource(index.data(pip::WindowSources::TokenRole).toString());
                    return;
                }
            }
            QCoreApplication::exit(1);
        });
        sourceStart.start(100);
    } else if (smokeTest) {
        QObject::connect(&smokeDeadline, &QTimer::timeout, &app, &QCoreApplication::quit);
        smokeDeadline.start(500);
    }
    const auto result = app.exec();
    return smokeTest && qmlFailed ? 1 : result;
}
