#pragma once
#include "capture/CaptureService.h"
#include "capture/WindowSources.h"
#include "core/SettingsStore.h"
#include "platform/GlobalShortcut.h"
#include <QMenu>
#include <QPointer>
#include <QSystemTrayIcon>
#include <QWindow>

namespace pip {
class AppController final : public QObject {
    Q_OBJECT
    Q_PROPERTY(pip::WindowSources* sources READ sources CONSTANT)
    Q_PROPERTY(bool active READ active NOTIFY changed)
    Q_PROPERTY(bool hasFrame READ hasFrame NOTIFY changed)
    Q_PROPERTY(bool locked READ locked NOTIFY changed)
    Q_PROPERTY(bool wayland READ wayland CONSTANT)
    Q_PROPERTY(bool dark READ dark NOTIFY changed)
    Q_PROPERTY(QString sourceTitle READ sourceTitle NOTIFY changed)
    Q_PROPERTY(QString message READ message NOTIFY changed)
    Q_PROPERTY(QString shortcut READ shortcut NOTIFY changed)
    Q_PROPERTY(QString shortcutStatus READ shortcutStatus NOTIFY changed)
    Q_PROPERTY(int scaleMode READ scaleMode WRITE setScaleMode NOTIFY changed)
    Q_PROPERTY(int theme READ theme WRITE setTheme NOTIFY changed)
    Q_PROPERTY(int opacity READ opacity WRITE setOpacity NOTIFY changed)
    Q_PROPERTY(bool preserveAspect READ preserveAspect WRITE setPreserveAspect NOTIFY changed)
    Q_PROPERTY(int pipWidth READ pipWidth NOTIFY geometryChanged)
    Q_PROPERTY(int pipHeight READ pipHeight NOTIFY geometryChanged)
public:
    explicit AppController(bool smokeTest = false, QObject* parent = nullptr);
    ~AppController() override;
    [[nodiscard]] WindowSources* sources();
    [[nodiscard]] bool active() const;
    [[nodiscard]] bool hasFrame() const;
    [[nodiscard]] bool locked() const;
    [[nodiscard]] bool wayland() const;
    [[nodiscard]] bool dark() const;
    [[nodiscard]] QString sourceTitle() const;
    [[nodiscard]] QString message() const;
    [[nodiscard]] QString shortcut() const;
    [[nodiscard]] QString shortcutStatus() const;
    [[nodiscard]] int scaleMode() const;
    [[nodiscard]] int theme() const;
    [[nodiscard]] int opacity() const;
    [[nodiscard]] bool preserveAspect() const;
    [[nodiscard]] int pipWidth() const;
    [[nodiscard]] int pipHeight() const;
    void setScaleMode(int value);
    void setTheme(int value);
    void setOpacity(int value);
    void setPreserveAspect(bool value);
    Q_INVOKABLE void attachOverlay(QWindow* window, QObject* videoSink);
    Q_INVOKABLE void selectSource(const QString& token);
    Q_INVOKABLE void chooseWithPortal();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void toggleLock();
    Q_INVOKABLE void unlock();
    Q_INVOKABLE void showControls();
    Q_INVOKABLE void showOverlay();
    Q_INVOKABLE void resizePip(int width, int height);
    Q_INVOKABLE bool setShortcut(const QString& text);
    Q_INVOKABLE void resetPreferences();
    Q_INVOKABLE void clearMessage();
    Q_INVOKABLE void quit();
signals:
    void changed();
    void geometryChanged();
    void controlsRequested();
private:
    void save();
    void applyWindowState();
    void geometryUpdated();
    void report(const QString& message);
    WindowSources sources_;
    CaptureService capture_;
    GlobalShortcut shortcut_;
    SettingsStore store_;
    Preferences preferences_;
    QPointer<QWindow> overlay_;
    QSystemTrayIcon tray_;
    QMenu trayMenu_;
    QTimer saveTimer_;
    QString message_;
    bool locked_{false};
    bool applyingGeometry_{false};
    bool smokeTest_{false};
    QRect lockedGeometry_;
};
}
