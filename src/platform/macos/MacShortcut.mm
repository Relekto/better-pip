#include "platform/GlobalShortcut.h"
#include <Carbon/Carbon.h>
#include <array>

namespace pip {
namespace {
class MacShortcut final : public ShortcutBackend {
  public:
    explicit MacShortcut(GlobalShortcut &owner) : owner_(owner) {
        const EventTypeSpec type{kEventClassKeyboard, kEventHotKeyPressed};
        InstallApplicationEventHandler(&MacShortcut::handle, 1, &type, this, &handler_);
    }
    ~MacShortcut() override {
        if (hotKey_) {
            UnregisterEventHotKey(hotKey_);
        }
        if (handler_) {
            RemoveEventHandler(handler_);
        }
    }
    bool bind(const QKeySequence &sequence, QString &error) override {
        const auto combination = sequence[0];
        const auto key = combination.key();
        constexpr std::array<UInt32, 26> letters{
            kVK_ANSI_A, kVK_ANSI_B, kVK_ANSI_C, kVK_ANSI_D, kVK_ANSI_E, kVK_ANSI_F, kVK_ANSI_G,
            kVK_ANSI_H, kVK_ANSI_I, kVK_ANSI_J, kVK_ANSI_K, kVK_ANSI_L, kVK_ANSI_M, kVK_ANSI_N,
            kVK_ANSI_O, kVK_ANSI_P, kVK_ANSI_Q, kVK_ANSI_R, kVK_ANSI_S, kVK_ANSI_T, kVK_ANSI_U,
            kVK_ANSI_V, kVK_ANSI_W, kVK_ANSI_X, kVK_ANSI_Y, kVK_ANSI_Z};
        constexpr std::array<UInt32, 12> functions{kVK_F1, kVK_F2,  kVK_F3,  kVK_F4,
                                                   kVK_F5, kVK_F6,  kVK_F7,  kVK_F8,
                                                   kVK_F9, kVK_F10, kVK_F11, kVK_F12};
        UInt32 code = 0;
        if (key >= Qt::Key_A && key <= Qt::Key_Z) {
            code = letters.at(static_cast<std::size_t>(key - Qt::Key_A));
        } else if (key >= Qt::Key_F1 && key <= Qt::Key_F12) {
            code = functions.at(static_cast<std::size_t>(key - Qt::Key_F1));
        } else if (key == Qt::Key_Space) {
            code = kVK_Space;
        } else {
            error = QStringLiteral("Choose a letter, F1–F12, or Space.");
            return false;
        }
        UInt32 modifiers = 0;
        const auto mods = combination.keyboardModifiers();
        if (mods.testFlag(Qt::ControlModifier)) {
            modifiers |= cmdKey;
        }
        if (mods.testFlag(Qt::MetaModifier)) {
            modifiers |= controlKey;
        }
        if (mods.testFlag(Qt::AltModifier)) {
            modifiers |= optionKey;
        }
        if (mods.testFlag(Qt::ShiftModifier)) {
            modifiers |= shiftKey;
        }
        if (code == code_ && modifiers == modifiers_ && hotKey_) {
            return true;
        }
        EventHotKeyRef next = nullptr;
        if (!handler_ || RegisterEventHotKey(code, modifiers, {0x42504950, 1},
                                             GetApplicationEventTarget(), 0, &next) != noErr) {
            error = QStringLiteral("That shortcut is reserved or already in use.");
            return false;
        }
        if (hotKey_) {
            UnregisterEventHotKey(hotKey_);
        }
        hotKey_ = next;
        code_ = code;
        modifiers_ = modifiers;
        return true;
    }

  private:
    static OSStatus handle(EventHandlerCallRef, EventRef event, void *context) {
        EventHotKeyID identifier{};
        if (GetEventParameter(event, kEventParamDirectObject, typeEventHotKeyID, nullptr,
                              sizeof(identifier), nullptr, &identifier) == noErr &&
            identifier.signature == 0x42504950) {
            emit static_cast<MacShortcut *>(context)->owner_.activated();
            return noErr;
        }
        return eventNotHandledErr;
    }
    GlobalShortcut &owner_;
    EventHandlerRef handler_{nullptr};
    EventHotKeyRef hotKey_{nullptr};
    UInt32 code_{0};
    UInt32 modifiers_{0};
};
}
std::unique_ptr<ShortcutBackend> makeShortcutBackend(GlobalShortcut &owner) {
    return std::make_unique<MacShortcut>(owner);
}
}
