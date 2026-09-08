#include "platform/GlobalShortcut.h"
#include <QAbstractNativeEventFilter>
#include <QCoreApplication>
#include <Windows.h>

namespace pip {
namespace {
class WindowsShortcut final : public ShortcutBackend, public QAbstractNativeEventFilter {
public:
    explicit WindowsShortcut(GlobalShortcut& owner) : owner_(owner) {
        QCoreApplication::instance()->installNativeEventFilter(this);
    }
    ~WindowsShortcut() override {
        if (id_ != 0)
            UnregisterHotKey(nullptr, id_);
        QCoreApplication::instance()->removeNativeEventFilter(this);
    }
    bool bind(const QKeySequence& sequence, QString& error) override {
        const auto combination = sequence[0];
        const auto key = combination.key();
        UINT nativeKey = 0;
        if ((key >= Qt::Key_A && key <= Qt::Key_Z) || (key >= Qt::Key_0 && key <= Qt::Key_9))
            nativeKey = static_cast<UINT>(key);
        else if (key >= Qt::Key_F1 && key <= Qt::Key_F24)
            nativeKey = VK_F1 + static_cast<UINT>(key - Qt::Key_F1);
        else if (key == Qt::Key_Space)
            nativeKey = VK_SPACE;
        else if (key == Qt::Key_Home)
            nativeKey = VK_HOME;
        else if (key == Qt::Key_End)
            nativeKey = VK_END;
        else if (key == Qt::Key_Insert)
            nativeKey = VK_INSERT;
        else if (key == Qt::Key_Delete)
            nativeKey = VK_DELETE;
        else if (key == Qt::Key_Pause)
            nativeKey = VK_PAUSE;
        if (nativeKey == 0) {
            error = QStringLiteral("Choose a letter, number, function key, Space, Home, End, Insert, Delete, or Pause.");
            return false;
        }
        UINT modifiers = MOD_NOREPEAT;
        const auto qtModifiers = combination.keyboardModifiers();
        if (qtModifiers.testFlag(Qt::ControlModifier)) modifiers |= MOD_CONTROL;
        if (qtModifiers.testFlag(Qt::AltModifier)) modifiers |= MOD_ALT;
        if (qtModifiers.testFlag(Qt::ShiftModifier)) modifiers |= MOD_SHIFT;
        if (qtModifiers.testFlag(Qt::MetaModifier)) modifiers |= MOD_WIN;
        if (nativeKey == key_ && modifiers == modifiers_ && id_ != 0)
            return true;
        const int next = id_ == 0x4250 ? 0x4251 : 0x4250;
        if (!RegisterHotKey(nullptr, next, modifiers, nativeKey)) {
            error = QStringLiteral("That shortcut is reserved or already used by another application.");
            return false;
        }
        if (id_ != 0)
            UnregisterHotKey(nullptr, id_);
        id_ = next;
        key_ = nativeKey;
        modifiers_ = modifiers;
        return true;
    }
    bool nativeEventFilter(const QByteArray&, void* message, qintptr*) override {
        const auto* event = static_cast<MSG*>(message);
        if (event->message == WM_HOTKEY && static_cast<int>(event->wParam) == id_) {
            emit owner_.activated();
            return true;
        }
        return false;
    }
private:
    GlobalShortcut& owner_;
    int id_{0};
    UINT key_{0};
    UINT modifiers_{0};
};
}
std::unique_ptr<ShortcutBackend> makeShortcutBackend(GlobalShortcut& owner) {
    return std::make_unique<WindowsShortcut>(owner);
}
}
