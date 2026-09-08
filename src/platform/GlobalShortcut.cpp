#include "GlobalShortcut.h"

namespace pip {
GlobalShortcut::GlobalShortcut(QObject* parent)
    : QObject(parent), backend_(makeShortcutBackend(*this)) {}
GlobalShortcut::~GlobalShortcut() = default;
bool GlobalShortcut::setSequence(const QString& text) {
    const auto sequence = QKeySequence::fromString(text.trimmed(), QKeySequence::PortableText);
    if (sequence.count() != 1 || sequence.isEmpty() ||
        sequence[0].keyboardModifiers() == Qt::NoModifier) {
        error_ = QStringLiteral("Use one key with Ctrl, Alt, Shift, or Meta.");
        return false;
    }
    if (!backend_->bind(sequence, error_))
        return false;
    sequence_ = sequence.toString(QKeySequence::PortableText);
    registered_ = true;
    error_.clear();
    return true;
}
QString GlobalShortcut::sequence() const { return sequence_; }
QString GlobalShortcut::error() const { return error_; }
bool GlobalShortcut::registered() const { return registered_; }
}
