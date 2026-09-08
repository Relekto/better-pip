#include "Preferences.h"
#include <algorithm>
#include <cmath>

namespace pip {
Preferences sanitized(Preferences value) {
    if (value.scaleMode < ScaleMode::Fit || value.scaleMode > ScaleMode::Stretch) {
        value.scaleMode = ScaleMode::Fit;
    }
    if (value.theme < Theme::System || value.theme > Theme::Dark) {
        value.theme = Theme::System;
    }
    value.opacity = std::clamp(value.opacity, 20, 100);
    value.geometry.setWidth(std::clamp(value.geometry.width(), 160, 8192));
    value.geometry.setHeight(std::clamp(value.geometry.height(), 90, 8192));
    if (value.lockShortcut.size() > 80) {
        value.lockShortcut = QStringLiteral("Ctrl+Alt+L");
    }
    return value;
}

QRect visibleGeometry(QRect requested, const QRect &available) {
    if (available.isEmpty()) {
        return requested;
    }
    requested.setWidth(std::clamp(requested.width(), 1, available.width()));
    requested.setHeight(std::clamp(requested.height(), 1, available.height()));
    requested.moveLeft(
        std::clamp(requested.left(), available.left(), available.right() - requested.width() + 1));
    requested.moveTop(
        std::clamp(requested.top(), available.top(), available.bottom() - requested.height() + 1));
    return requested;
}

QSize constrainedSize(QSize requested, double aspect) {
    requested = requested.expandedTo(QSize(160, 90)).boundedTo(QSize(8192, 8192));
    if (!std::isfinite(aspect) || aspect < 160.0 / 8192.0 || aspect > 8192.0 / 90.0) {
        return requested;
    }
    const double width =
        std::clamp(static_cast<double>(requested.width()), std::max(160.0, 90.0 * aspect),
                   std::min(8192.0, 8192.0 * aspect));
    return {qRound(width), qRound(width / aspect)};
}
}
