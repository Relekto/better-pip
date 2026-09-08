#include "platform/GlobalShortcut.h"
#include "PortalShortcut.h"
#include <QGuiApplication>
#include <QSocketNotifier>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <array>

namespace pip {
namespace {
bool grabFailed = false;
int captureGrabError(Display*, XErrorEvent*) {
    grabFailed = true;
    return 0;
}
class LinuxShortcut final : public ShortcutBackend {
public:
    explicit LinuxShortcut(GlobalShortcut& owner) : owner_(owner) {
        if (!QGuiApplication::platformName().contains(QStringLiteral("wayland"))) {
            display_ = XOpenDisplay(nullptr);
            if (display_) {
                notifier_ = std::make_unique<QSocketNotifier>(ConnectionNumber(display_),
                                                              QSocketNotifier::Read);
                QObject::connect(notifier_.get(), &QSocketNotifier::activated, &owner_,
                                 [this] { drain(); });
            }
        }
    }
    ~LinuxShortcut() override {
        notifier_.reset();
        if (display_) {
            release(key_, modifiers_);
            XCloseDisplay(display_);
        }
    }
    bool bind(const QKeySequence& sequence, QString& error) override {
        if (!display_) {
            error = QStringLiteral("Global shortcuts are unavailable in this session. Use the control window to unlock.");
            return false;
        }
        const auto combination = sequence[0];
        const auto name = QKeySequence(combination.key()).toString(QKeySequence::PortableText).toLatin1();
        const auto symbol = combination.key() == Qt::Key_Space ? XK_space : XStringToKeysym(name.constData());
        const auto code = XKeysymToKeycode(display_, symbol);
        if (!code) {
            error = QStringLiteral("This key cannot be registered on the current keyboard.");
            return false;
        }
        unsigned int modifiers = 0;
        const auto mods = combination.keyboardModifiers();
        if (mods.testFlag(Qt::ControlModifier)) modifiers |= ControlMask;
        if (mods.testFlag(Qt::AltModifier)) modifiers |= Mod1Mask;
        if (mods.testFlag(Qt::ShiftModifier)) modifiers |= ShiftMask;
        if (mods.testFlag(Qt::MetaModifier)) modifiers |= Mod4Mask;
        if (code == key_ && modifiers == modifiers_)
            return true;
        XSync(display_, False);
        grabFailed = false;
        const auto previous = XSetErrorHandler(captureGrabError);
        for (auto extra : ignoredModifiers_)
            XGrabKey(display_, code, modifiers | extra, DefaultRootWindow(display_),
                     False, GrabModeAsync, GrabModeAsync);
        XSync(display_, False);
        XSetErrorHandler(previous);
        if (grabFailed) {
            release(code, modifiers);
            error = QStringLiteral("That shortcut is reserved or already in use.");
            return false;
        }
        release(key_, modifiers_);
        key_ = code;
        modifiers_ = modifiers;
        return true;
    }
private:
    void release(int key, unsigned int modifiers) {
        if (key == 0) return;
        for (auto extra : ignoredModifiers_)
            XUngrabKey(display_, key, modifiers | extra, DefaultRootWindow(display_));
        XFlush(display_);
    }
    void drain() {
        while (XPending(display_)) {
            XEvent event{};
            XNextEvent(display_, &event);
            if (event.type == KeyPress && event.xkey.keycode == key_)
                emit owner_.activated();
        }
    }
    GlobalShortcut& owner_;
    Display* display_{nullptr};
    std::unique_ptr<QSocketNotifier> notifier_;
    int key_{0};
    unsigned int modifiers_{0};
    const std::array<unsigned int, 4> ignoredModifiers_{0, LockMask, Mod2Mask, LockMask | Mod2Mask};
};
}
std::unique_ptr<ShortcutBackend> makeShortcutBackend(GlobalShortcut& owner) {
    if (QGuiApplication::platformName().contains(QStringLiteral("wayland")))
        return std::make_unique<PortalShortcut>(owner);
    return std::make_unique<LinuxShortcut>(owner);
}
}
