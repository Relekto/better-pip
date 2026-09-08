#include "AppController.h"
#include <QApplication>
#include <QGuiApplication>
#include <QIcon>
#include <QScreen>
#include <QScopedValueRollback>
#include <QStandardPaths>
#include <QStyleHints>
#include <algorithm>

namespace pip {
AppController::AppController(bool smokeTest, QObject* parent)
    : QObject(parent),
      store_(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) +
             QStringLiteral("/settings.json")),
      preferences_(smokeTest ? Preferences{} : store_.load()), smokeTest_(smokeTest) {
    connect(&capture_, &CaptureService::changed, this, [this] {
        if (!capture_.active())
            locked_ = false;
        applyWindowState();
        emit changed();
    });
    connect(&capture_, &CaptureService::failed, this, &AppController::report);
    connect(&capture_, &CaptureService::frameSizeChanged, this, [this] {
        if (preferences_.preserveAspect && overlay_ && !locked_)
            resizePip(overlay_->width(), overlay_->height());
        emit changed();
    });
    connect(&shortcut_, &GlobalShortcut::activated, this, &AppController::toggleLock);
    connect(&shortcut_, &GlobalShortcut::registrationChanged, this, [this] {
        if (shortcut_.registered() && !shortcut_.pending()) {
            preferences_.lockShortcut = shortcut_.sequence();
            save();
        }
        if (!shortcut_.error().isEmpty()) message_ = shortcut_.error();
        emit changed();
    });
    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged,
            this, [this] { emit changed(); });
    connect(qGuiApp, &QGuiApplication::screenRemoved, this, [this](QScreen*) {
        if (overlay_ && QGuiApplication::primaryScreen()) {
            const auto available = QGuiApplication::primaryScreen()->availableGeometry();
            lockedGeometry_ = visibleGeometry(overlay_->geometry(), available);
            QScopedValueRollback guard(applyingGeometry_, true);
            overlay_->setGeometry(lockedGeometry_);
            preferences_.geometry = lockedGeometry_;
            save();
        }
    });
    saveTimer_.setSingleShot(true);
    saveTimer_.setInterval(350);
    connect(&saveTimer_, &QTimer::timeout, this, [this] {
        QString error;
        if (!smokeTest_ && !store_.save(preferences_, &error))
            report(QStringLiteral("Settings could not be saved: ") + error);
    });
    if (!smokeTest_ && !shortcut_.setSequence(preferences_.lockShortcut))
        message_ = shortcut_.error();
    tray_.setIcon(QIcon(QStringLiteral(":/assets/icon.svg")));
    tray_.setToolTip(QStringLiteral("Better PiP"));
    trayMenu_.addAction(QStringLiteral("Open controls / unlock"), this, &AppController::showControls);
    trayMenu_.addAction(QStringLiteral("Lock / unlock PiP"), this, &AppController::toggleLock);
    trayMenu_.addAction(QStringLiteral("Stop picture-in-picture"), this, &AppController::stop);
    trayMenu_.addSeparator();
    trayMenu_.addAction(QStringLiteral("Quit Better PiP"), this, &AppController::quit);
    tray_.setContextMenu(&trayMenu_);
    connect(&tray_, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick)
            showControls();
    });
    if (!smokeTest_ && QSystemTrayIcon::isSystemTrayAvailable())
        tray_.show();
    if (!smokeTest_)
        QTimer::singleShot(0, &sources_, &WindowSources::refresh);
}
AppController::~AppController() {
    QObject::disconnect(&capture_, nullptr, this, nullptr);
    QObject::disconnect(&shortcut_, nullptr, this, nullptr);
    tray_.hide();
    tray_.setContextMenu(nullptr);
    saveTimer_.stop();
    capture_.stop();
    if (!smokeTest_)
        static_cast<void>(store_.save(preferences_));
}
WindowSources* AppController::sources() { return &sources_; }
bool AppController::active() const { return capture_.active(); }
bool AppController::hasFrame() const { return capture_.hasFrame(); }
bool AppController::locked() const { return locked_; }
bool AppController::wayland() const {
    return QGuiApplication::platformName().contains(QStringLiteral("wayland"));
}
bool AppController::dark() const {
    return preferences_.theme == Theme::Dark ||
           (preferences_.theme == Theme::System &&
            QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark);
}
QString AppController::sourceTitle() const { return capture_.title(); }
QString AppController::message() const { return message_; }
QString AppController::shortcut() const {
    return wayland() && shortcut_.registered() ? shortcut_.description() : preferences_.lockShortcut;
}
QString AppController::shortcutStatus() const {
    if (shortcut_.pending()) return QStringLiteral("Waiting for your desktop's shortcut permission…");
    return shortcut_.registered() ? QStringLiteral("Global shortcut active")
                                  : QStringLiteral("Use Open controls to unlock");
}
int AppController::scaleMode() const { return static_cast<int>(preferences_.scaleMode); }
int AppController::theme() const { return static_cast<int>(preferences_.theme); }
int AppController::opacity() const { return preferences_.opacity; }
bool AppController::preserveAspect() const { return preferences_.preserveAspect; }
int AppController::pipWidth() const { return overlay_ ? overlay_->width() : preferences_.geometry.width(); }
int AppController::pipHeight() const { return overlay_ ? overlay_->height() : preferences_.geometry.height(); }
void AppController::setScaleMode(int value) {
    if (value < 0 || value > 2 || scaleMode() == value) return;
    preferences_.scaleMode = static_cast<ScaleMode>(value);
    save();
    emit changed();
}
void AppController::setTheme(int value) {
    if (value < 0 || value > 2 || theme() == value) return;
    preferences_.theme = static_cast<Theme>(value);
    save();
    emit changed();
}
void AppController::setOpacity(int value) {
    value = std::clamp(value, 20, 100);
    if (value == opacity()) return;
    preferences_.opacity = value;
    if (overlay_) overlay_->setOpacity(static_cast<qreal>(value) / 100.0);
    save();
    emit changed();
}
void AppController::setPreserveAspect(bool value) {
    if (value == preserveAspect()) return;
    preferences_.preserveAspect = value;
    if (value && !locked_) resizePip(pipWidth(), pipHeight());
    save();
    emit changed();
}
void AppController::attachOverlay(QWindow* window, QObject* videoSink) {
    overlay_ = window;
    capture_.setVideoSink(qobject_cast<QVideoSink*>(videoSink));
    if (!window) return;
    window->setMinimumSize(QSize(160, 90));
    window->setMaximumSize(QSize(8192, 8192));
    auto geometry = preferences_.geometry;
    QScreen* screen = nullptr;
    for (auto* candidate : QGuiApplication::screens())
        if (candidate->availableGeometry().contains(geometry.center()))
            screen = candidate;
    if (!screen) screen = QGuiApplication::primaryScreen();
    if (screen) geometry = visibleGeometry(geometry, screen->availableGeometry());
    window->setGeometry(geometry);
    window->setOpacity(static_cast<qreal>(opacity()) / 100.0);
    connect(window, &QWindow::xChanged, this, &AppController::geometryUpdated);
    connect(window, &QWindow::yChanged, this, &AppController::geometryUpdated);
    connect(window, &QWindow::widthChanged, this, &AppController::geometryUpdated);
    connect(window, &QWindow::heightChanged, this, &AppController::geometryUpdated);
    applyWindowState();
}
void AppController::selectSource(const QString& token) {
    clearMessage();
    unlock();
    capture_.start(sources_.window(token));
}
void AppController::chooseWithPortal() {
    clearMessage();
    unlock();
    capture_.startPortal();
}
void AppController::stop() {
    unlock();
    capture_.stop();
}
void AppController::toggleLock() {
    if (!capture_.active() || !overlay_) {
        showControls();
        return;
    }
    locked_ = !locked_;
    if (locked_) lockedGeometry_ = overlay_->geometry();
    applyWindowState();
    emit changed();
}
void AppController::unlock() {
    if (!locked_) return;
    locked_ = false;
    applyWindowState();
    emit changed();
}
void AppController::showControls() {
    unlock();
    emit controlsRequested();
}
void AppController::showOverlay() {
    if (overlay_ && active()) {
        overlay_->show();
        overlay_->raise();
    }
}
void AppController::resizePip(int width, int height) {
    if (!overlay_ || locked_) return;
    auto size = QSize(width, height).expandedTo(QSize(160, 90)).boundedTo(QSize(8192, 8192));
    const auto sourceSize = capture_.frameSize();
    if (preferences_.preserveAspect && !sourceSize.isEmpty())
        size = constrainedSize(size, static_cast<double>(sourceSize.width()) / sourceSize.height());
    overlay_->resize(size);
}
bool AppController::setShortcut(const QString& text) {
    if (!shortcut_.setSequence(text)) {
        report(shortcut_.error());
        return false;
    }
    if (!shortcut_.pending()) preferences_.lockShortcut = shortcut_.sequence();
    save();
    clearMessage();
    emit changed();
    return true;
}
void AppController::resetPreferences() {
    unlock();
    Preferences defaults;
    if (!shortcut_.setSequence(defaults.lockShortcut))
        defaults.lockShortcut = preferences_.lockShortcut;
    preferences_ = defaults;
    if (overlay_) {
        auto geometry = defaults.geometry;
        if (auto* screen = QGuiApplication::primaryScreen())
            geometry = visibleGeometry(geometry, screen->availableGeometry());
        overlay_->setGeometry(geometry);
        overlay_->setOpacity(1);
    }
    save();
    emit changed();
}
void AppController::clearMessage() {
    if (message_.isEmpty()) return;
    message_.clear();
    emit changed();
}
void AppController::quit() {
    stop();
    QCoreApplication::quit();
}
void AppController::save() {
    if (!smokeTest_) saveTimer_.start();
}
void AppController::applyWindowState() {
    if (!overlay_) return;
    const auto geometry = overlay_->geometry();
    Qt::WindowFlags flags = Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint;
    if (locked_) flags |= Qt::WindowTransparentForInput | Qt::WindowDoesNotAcceptFocus;
    QScopedValueRollback guard(applyingGeometry_, true);
    if (overlay_->flags() != flags) overlay_->setFlags(flags);
    overlay_->setGeometry(geometry);
    overlay_->setVisible(capture_.active());
}
void AppController::geometryUpdated() {
    if (!overlay_ || applyingGeometry_) return;
    QScopedValueRollback guard(applyingGeometry_, true);
    if (locked_) {
        overlay_->setGeometry(lockedGeometry_);
        return;
    }
    const auto sourceSize = capture_.frameSize();
    if (preferences_.preserveAspect && !sourceSize.isEmpty())
        overlay_->resize(constrainedSize(overlay_->size(),
                         static_cast<double>(sourceSize.width()) / sourceSize.height()));
    preferences_.geometry = overlay_->geometry();
    save();
    emit geometryChanged();
}
void AppController::report(const QString& message) {
    message_ = message;
    emit changed();
}
}
