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
    if (backend_->asynchronous()) {
        pendingSequence_ = sequence.toString(QKeySequence::PortableText);
        error_.clear();
        emit registrationChanged();
        return true;
    }
    sequence_ = sequence.toString(QKeySequence::PortableText);
    registered_ = true;
    error_.clear();
    description_ = sequence_;
    emit registrationChanged();
    return true;
}
QString GlobalShortcut::sequence() const { return sequence_; }
QString GlobalShortcut::error() const { return error_; }
bool GlobalShortcut::registered() const { return registered_; }
QString GlobalShortcut::description() const { return description_; }
bool GlobalShortcut::pending() const { return !pendingSequence_.isEmpty(); }
void GlobalShortcut::completeRegistration(bool success, const QString& error, const QString& description) {
    if (success) {
        sequence_ = pendingSequence_;
        description_ = description.isEmpty() ? sequence_ : description;
        registered_ = true;
    }
    pendingSequence_.clear();
    error_ = error;
    emit registrationChanged();
}
void GlobalShortcut::invalidate(const QString& error) {
    registered_ = false;
    error_ = error;
    emit registrationChanged();
}
}
