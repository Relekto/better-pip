#include "MotionWindow.h"
#include "app/AppController.h"
#include <QApplication>
#include <QDir>
#include <QPainter>
#include <QProcess>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QTimer>
#include <array>

namespace {
constexpr int frameCount = 450;
const QString sourceTitle = QStringLiteral("Motion study - Better PiP studio");

bool safeSources(pip::WindowSources &sources) {
    for (int row = 0; row < sources.rowCount(); ++row) {
        if (sources.index(row, 0).data(pip::WindowSources::TitleRole).toString() != sourceTitle) {
            return false;
        }
    }
    return true;
}

QImage compose(const QImage &capture, int scene, int frame) {
    QImage image(1280, 900, QImage::Format_RGB32);
    image.fill(QColor("#0c0f15"));
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setPen(QColor("#79a8ff"));
    painter.setFont(QFont(QStringLiteral("Segoe UI"), 12, QFont::DemiBold));
    painter.drawText(54, 48, QStringLiteral("BETTER PiP"));
    const std::array<QString, 5> titles{
        QStringLiteral("Your window. Within reach."), QStringLiteral("Keep the whole picture."),
        QStringLiteral("Stretch it to your space."), QStringLiteral("Lock it. Keep working."),
        QStringLiteral("Make it feel like yours.")};
    const std::array<QString, 5> subtitles{
        QStringLiteral("Search a window title and start picture-in-picture."),
        QStringLiteral("Live capture with Fit, Fill and independent dimensions."),
        QStringLiteral("Wide or tall. Choose Stretch to fill the frame."),
        QStringLiteral("Locked PiP passes clicks through. Use your shortcut to unlock."),
        QStringLiteral("Light and dark themes, custom shortcuts and precise sizing.")};
    painter.setPen(QColor("#f0f2f7"));
    painter.setFont(QFont(QStringLiteral("Segoe UI"), 26, QFont::DemiBold));
    painter.drawText(54, 103, titles.at(static_cast<std::size_t>(scene)));
    painter.setPen(QColor("#a2aaba"));
    painter.setFont(QFont(QStringLiteral("Segoe UI"), 12));
    painter.drawText(54, 140, subtitles.at(static_cast<std::size_t>(scene)));
    QSize size = capture.size();
    size.scale(1050, 665, Qt::KeepAspectRatio);
    const QRect target((1280 - size.width()) / 2, 165 + (665 - size.height()) / 2, size.width(),
                       size.height());
    painter.drawImage(target, capture);
    painter.setPen(QColor("#778092"));
    painter.setFont(QFont(QStringLiteral("Segoe UI"), 10));
    painter.drawText(54, 866, QStringLiteral("AUTOMATED DEMO  /  REAL APP CAPTURE  /  WINDOWS"));
    painter.drawText(1118, 866, QStringLiteral("%1 / 05").arg(scene + 1, 2, 10, QLatin1Char('0')));
    painter.fillRect(QRect(54, 883, 1172 * (frame + 1) / frameCount, 3), QColor("#79a8ff"));
    return image;
}
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    if (app.arguments().contains(QStringLiteral("--source"))) {
        return runMotionWindow();
    }
    if (app.arguments().size() != 2) {
        qCritical("Usage: pip-studio OUTPUT_DIRECTORY");
        return 1;
    }
    const QDir output(app.arguments().at(1));
    if (!QDir().mkpath(output.filePath(QStringLiteral("frames")))) {
        return 1;
    }
    QCoreApplication::setApplicationName(QStringLiteral("Better PiP media studio"));
    QQuickStyle::setStyle(QStringLiteral("Basic"));
    QApplication::setQuitOnLastWindowClosed(false);
    QProcess source;
    source.start(QCoreApplication::applicationFilePath(), {QStringLiteral("--source")});
    if (!source.waitForStarted()) {
        return 1;
    }
    pip::AppController controller(true);
    controller.setTheme(2);
    controller.sources()->setFilter(QStringLiteral("Better PiP studio"));
    QQmlApplicationEngine engine;
    bool qmlFailed = false;
    QObject::connect(&engine, &QQmlApplicationEngine::warnings, &app,
                     [&qmlFailed] { qmlFailed = true; });
    engine.setInitialProperties({{QStringLiteral("controller"), QVariant::fromValue(&controller)}});
    engine.load(QUrl(QStringLiteral("qrc:/ui/Main.qml")));
    if (engine.rootObjects().isEmpty()) {
        return 1;
    }
    auto *main = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
    QQuickWindow *overlay = nullptr;
    for (auto *window : QGuiApplication::topLevelWindows()) {
        if (window != main) {
            if (auto *quick = qobject_cast<QQuickWindow *>(window)) {
                overlay = quick;
            }
        }
    }
    if (!main || !overlay) {
        return 1;
    }
    main->setPosition(680, 40);
    QTimer startup;
    startup.setSingleShot(true);
    QTimer timer;
    timer.setTimerType(Qt::PreciseTimer);
    timer.setInterval(34);
    int frame = 0;
    QObject::connect(&startup, &QTimer::timeout, &app, [&] {
        controller.sources()->refresh();
        if (!safeSources(*controller.sources()) || controller.sources()->count() != 1) {
            qCritical("The controlled source is unavailable");
            app.exit(1);
            return;
        }
        timer.start();
    });
    QObject::connect(&timer, &QTimer::timeout, &app, [&] {
        if (qmlFailed || !safeSources(*controller.sources())) {
            app.exit(1);
            return;
        }
        if (frame == 75) {
            controller.selectSource(
                controller.sources()->index(0, 0).data(pip::WindowSources::TokenRole).toString());
            overlay->setPosition(30, 480);
        }
        if (frame >= 90 && frame < 360 && !controller.hasFrame()) {
            qCritical("No live capture frame");
            app.exit(1);
            return;
        }
        if (frame == 180) {
            controller.setPreserveAspect(false);
            controller.setScaleMode(2);
        }
        if (frame >= 180 && frame < 270) {
            const double progress = static_cast<double>(frame - 180) / 89.0;
            controller.resizePip(static_cast<int>(900 - 560 * progress),
                                 static_cast<int>(300 + 270 * progress));
        }
        if (frame == 270) {
            controller.resizePip(740, 440);
            controller.setScaleMode(0);
            controller.toggleLock();
        }
        if (frame == 360) {
            controller.unlock();
            controller.stop();
            main->setProperty("settingsOpen", true);
        }
        if (frame == 405) {
            controller.setTheme(1);
        }
        const int scene = frame / 90;
        auto *window = scene > 0 && scene < 4 ? overlay : main;
        const QImage capture = window->grabWindow();
        if (capture.isNull()) {
            app.exit(1);
            return;
        }
        const auto image = compose(capture, scene, frame);
        const QString name = QStringLiteral("frames/%1.png").arg(frame, 4, 10, QLatin1Char('0'));
        if (!image.save(output.filePath(name), "PNG", 100)) {
            app.exit(1);
            return;
        }
        if (frame == 30 || frame == 195 || frame == 430) {
            if (!image.save(output.filePath(QStringLiteral("screenshot-%1.png").arg(scene)))) {
                app.exit(1);
                return;
            }
        }
        if (++frame == frameCount) {
            timer.stop();
            app.quit();
        }
    });
    startup.start(2000);
    const int result = app.exec();
    source.terminate();
    if (!source.waitForFinished(3000)) {
        source.kill();
        source.waitForFinished();
    }
    return result;
}
