#include "core/Preferences.h"
#include "core/SettingsStore.h"
#include <QFile>
#include <QTemporaryDir>
#include <QtTest>
#include <limits>

class PreferencesTest final : public QObject {
    Q_OBJECT
  private slots:
    void clampsInvalidInput() {
        pip::Preferences value;
        value.opacity = -20;
        value.geometry = QRect(0, 0, -1, 1000000);
        value.theme = static_cast<pip::Theme>(80);
        const auto result = pip::sanitized(value);
        QCOMPARE(result.opacity, 20);
        QCOMPARE(result.geometry.size(), QSize(160, 8192));
        QCOMPARE(result.theme, pip::Theme::System);
    }
    void recoversRemovedMonitor() {
        const auto geometry =
            pip::visibleGeometry(QRect(6000, -4000, 640, 360), QRect(-1920, 0, 1920, 1080));
        QVERIFY(QRect(-1920, 0, 1920, 1080).contains(geometry));
        QCOMPARE(geometry.size(), QSize(640, 360));
    }
    void preservesAspect() {
        QCOMPARE(pip::constrainedSize(QSize(800, 400), 16.0 / 9.0), QSize(800, 450));
        QCOMPARE(pip::constrainedSize(QSize(200, 400), 0.5), QSize(200, 400));
        QCOMPARE(pip::constrainedSize(QSize(800, 400), 0), QSize(800, 400));
        QCOMPARE(pip::constrainedSize(QSize(800, 400), std::numeric_limits<double>::quiet_NaN()),
                 QSize(800, 400));
    }
    void roundTrip() {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        pip::SettingsStore store(directory.filePath(QStringLiteral("settings.json")));
        pip::Preferences value;
        value.scaleMode = pip::ScaleMode::Stretch;
        value.opacity = 72;
        value.geometry = QRect(-600, 20, 400, 800);
        value.lockShortcut = QStringLiteral("Ctrl+Shift+F8");
        QVERIFY(store.save(value));
        const auto loaded = store.load();
        QCOMPARE(loaded.scaleMode, value.scaleMode);
        QCOMPARE(loaded.opacity, value.opacity);
        QCOMPARE(loaded.geometry, value.geometry);
        QCOMPARE(loaded.lockShortcut, value.lockShortcut);
    }
    void rejectsCorruptionAndFutureVersions() {
        QTemporaryDir directory;
        const auto path = directory.filePath(QStringLiteral("settings.json"));
        pip::SettingsStore store(path);
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("{broken");
        file.close();
        QCOMPARE(store.load().opacity, 100);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
        file.write(R"({"version":999,"opacity":20})");
        file.close();
        QCOMPARE(store.load().opacity, 100);
    }
};
QTEST_GUILESS_MAIN(PreferencesTest)
#include "PreferencesTest.moc"
