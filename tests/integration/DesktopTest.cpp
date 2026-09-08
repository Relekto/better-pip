#include "app/AppController.h"
#include "platform/GlobalShortcut.h"
#include <QProcess>
#include <QSignalSpy>
#include <QUuid>
#include <QVideoSink>
#include <QVideoFrame>
#include <QtTest>
#ifdef Q_OS_WIN
#include <Windows.h>
#endif

class DesktopTest final : public QObject {
    Q_OBJECT
private slots:
    void captureResizeAndLock() {
        const auto title = QStringLiteral("Better PiP fixture ") + QUuid::createUuid().toString();
        QProcess fixture;
        fixture.start(QCoreApplication::applicationDirPath() + QStringLiteral("/source-fixture"), {title});
        QVERIFY(fixture.waitForStarted());
        struct StopFixture {
            QProcess& process;
            ~StopFixture() {
                process.terminate();
                if (!process.waitForFinished(2000)) {
                    process.kill();
                    process.waitForFinished(2000);
                }
            }
        } stopFixture{fixture};
        pip::AppController controller(true);
        QWindow overlay;
        overlay.setTitle(QStringLiteral("Better PiP test overlay"));
        overlay.setFlag(Qt::WindowDoesNotAcceptFocus);
        QVideoSink sink;
        controller.attachOverlay(&overlay, &sink);
        QString token;
        const auto findSource = [&] {
            controller.sources()->refresh();
            for (int row = 0; row < controller.sources()->rowCount(); ++row) {
                const auto index = controller.sources()->index(row, 0);
                if (index.data(pip::WindowSources::TitleRole).toString() == title) {
                    token = index.data(pip::WindowSources::TokenRole).toString();
                    return true;
                }
            }
            return false;
        };
        QTRY_VERIFY_WITH_TIMEOUT(findSource(), 6000);
        controller.selectSource(token);
        QTRY_VERIFY_WITH_TIMEOUT(controller.hasFrame() || !controller.message().isEmpty(), 10000);
        QVERIFY2(controller.hasFrame(), qPrintable(controller.message()));
        QVERIFY(sink.videoFrame().isValid());
        const auto image = sink.videoFrame().toImage();
        QVERIFY(!image.isNull());
        const auto sample = image.pixelColor(image.width() / 3, image.height() / 2);
        QVERIFY(sample.green() > sample.blue());
        controller.resizePip(700, 200);
        QCOMPARE(overlay.size(), QSize(700, 200));
        controller.setScaleMode(2);
        QCOMPARE(controller.scaleMode(), 2);
        controller.setPreserveAspect(true);
        QVERIFY(overlay.height() > 200);
        controller.setPreserveAspect(false);
        controller.toggleLock();
        QVERIFY(controller.locked());
        QVERIFY(overlay.flags().testFlag(Qt::WindowTransparentForInput));
        const auto lockedGeometry = overlay.geometry();
        controller.resizePip(900, 600);
        QCOMPARE(overlay.geometry(), lockedGeometry);
#ifdef Q_OS_WIN
        const auto handle = reinterpret_cast<HWND>(overlay.winId());
        const auto center = overlay.mapToGlobal(QPoint(40, 40));
        const POINT location{center.x(), center.y()};
        QTRY_VERIFY_WITH_TIMEOUT(WindowFromPoint(location) != handle, 2000);
        controller.unlock();
        QTRY_COMPARE_WITH_TIMEOUT(WindowFromPoint(location), handle, 2000);
#else
        controller.unlock();
#endif
        QVERIFY(!controller.locked());
        for (int iteration = 0; iteration < 100; ++iteration) {
            controller.toggleLock();
            QVERIFY(controller.locked());
            controller.unlock();
            QVERIFY(!controller.locked());
        }
        controller.stop();
        QVERIFY(!controller.active());
        QVERIFY(!overlay.isVisible());
        QVERIFY(!sink.videoFrame().isValid());
    }
    void shortcutConflictPreservesBinding() {
#ifdef Q_OS_WIN
        pip::GlobalShortcut shortcut;
        QVERIFY(shortcut.setSequence(QStringLiteral("Ctrl+Alt+F24")));
        QVERIFY(RegisterHotKey(nullptr, 0x4260, MOD_CONTROL | MOD_ALT, VK_F23));
        struct ReleaseHotkey {
            ~ReleaseHotkey() { UnregisterHotKey(nullptr, 0x4260); }
        } release;
        QVERIFY(!shortcut.setSequence(QStringLiteral("Ctrl+Alt+F23")));
        QCOMPARE(shortcut.sequence(), QStringLiteral("Ctrl+Alt+F24"));
        QVERIFY(shortcut.registered());
        QVERIFY(!shortcut.error().isEmpty());
        QVERIFY(!shortcut.setSequence(QStringLiteral("A")));
        QCOMPARE(shortcut.sequence(), QStringLiteral("Ctrl+Alt+F24"));
#endif
    }
};
QTEST_MAIN(DesktopTest)
#include "DesktopTest.moc"
