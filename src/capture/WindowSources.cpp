#include "WindowSources.h"
#include <QGuiApplication>
#include <QUuid>
#include <QWindow>
#include <QWindowCapture>
#include <algorithm>

namespace pip {
WindowSources::WindowSources(QObject* parent) : QAbstractListModel(parent) {
    refreshTimer_.setInterval(2500);
    connect(&refreshTimer_, &QTimer::timeout, this, &WindowSources::refresh);
    refreshTimer_.start();
}
int WindowSources::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : static_cast<int>(filtered_.size());
}
QVariant WindowSources::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= filtered_.size())
        return {};
    const auto& source = filtered_.at(index.row());
    if (role == TitleRole)
        return source.window.description();
    if (role == TokenRole)
        return source.token;
    return {};
}
QHash<int, QByteArray> WindowSources::roleNames() const {
    return {{TitleRole, "windowTitle"}, {TokenRole, "sourceToken"}};
}
QString WindowSources::filter() const { return filter_; }
int WindowSources::count() const { return static_cast<int>(filtered_.size()); }
void WindowSources::setFilter(const QString& filter) {
    if (filter_ == filter)
        return;
    filter_ = filter;
    rebuild();
    emit filterChanged();
}
QCapturableWindow WindowSources::window(const QString& token) const {
    for (const auto& source : sources_)
        if (source.token == token)
            return source.window;
    return {};
}
void WindowSources::refresh() {
    if (QGuiApplication::platformName().contains(QStringLiteral("wayland")))
        return;
    QList<QCapturableWindow> ownWindows;
    for (auto* window : QGuiApplication::topLevelWindows())
        ownWindows.append(QCapturableWindow(window));
    QList<Source> next;
    for (const auto& window : QWindowCapture::capturableWindows()) {
        if (ownWindows.contains(window) || window.description().trimmed().isEmpty())
            continue;
        const auto found = std::find_if(sources_.cbegin(), sources_.cend(),
                                       [&window](const Source& item) { return item.window == window; });
        next.append({window, found == sources_.cend()
                                ? QUuid::createUuid().toString(QUuid::WithoutBraces)
                                : found->token});
    }
    std::sort(next.begin(), next.end(), [](const Source& a, const Source& b) {
        return QString::localeAwareCompare(a.window.description(), b.window.description()) < 0;
    });
    sources_ = std::move(next);
    rebuild();
}
void WindowSources::rebuild() {
    beginResetModel();
    filtered_.clear();
    for (const auto& source : sources_)
        if (source.window.description().contains(filter_.trimmed(), Qt::CaseInsensitive))
            filtered_.append(source);
    endResetModel();
    emit countChanged();
}
}
