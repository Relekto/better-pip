#include "SettingsStore.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

namespace pip {
SettingsStore::SettingsStore(QString path) : path_(std::move(path)) {}

Preferences SettingsStore::load() const {
    QFile file(path_);
    if (!file.open(QIODevice::ReadOnly) || file.size() > 65536)
        return {};
    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError || !document.isObject())
        return {};
    const auto object = document.object();
    if (object.value(QStringLiteral("version")).toInt() != 1)
        return {};
    Preferences value;
    value.scaleMode = static_cast<ScaleMode>(object.value(QStringLiteral("scaleMode")).toInt());
    value.theme = static_cast<Theme>(object.value(QStringLiteral("theme")).toInt());
    value.opacity = object.value(QStringLiteral("opacity")).toInt(100);
    value.preserveAspect = object.value(QStringLiteral("preserveAspect")).toBool();
    value.lockShortcut = object.value(QStringLiteral("lockShortcut")).toString(value.lockShortcut);
    const auto geometry = object.value(QStringLiteral("geometry")).toObject();
    value.geometry = QRect(geometry.value(QStringLiteral("x")).toInt(80),
                           geometry.value(QStringLiteral("y")).toInt(80),
                           geometry.value(QStringLiteral("width")).toInt(640),
                           geometry.value(QStringLiteral("height")).toInt(360));
    return sanitized(std::move(value));
}

bool SettingsStore::save(const Preferences& preferences, QString* error) const {
    const auto value = sanitized(preferences);
    if (!QDir().mkpath(QFileInfo(path_).absolutePath())) {
        if (error)
            *error = QStringLiteral("Could not create the settings directory.");
        return false;
    }
    const QJsonObject geometry{{QStringLiteral("x"), value.geometry.x()},
                               {QStringLiteral("y"), value.geometry.y()},
                               {QStringLiteral("width"), value.geometry.width()},
                               {QStringLiteral("height"), value.geometry.height()}};
    const QJsonObject object{{QStringLiteral("version"), 1},
                             {QStringLiteral("scaleMode"), static_cast<int>(value.scaleMode)},
                             {QStringLiteral("theme"), static_cast<int>(value.theme)},
                             {QStringLiteral("opacity"), value.opacity},
                             {QStringLiteral("preserveAspect"), value.preserveAspect},
                             {QStringLiteral("lockShortcut"), value.lockShortcut},
                             {QStringLiteral("geometry"), geometry}};
    QSaveFile file(path_);
    const auto bytes = QJsonDocument(object).toJson();
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit()) {
        if (error)
            *error = file.errorString();
        return false;
    }
    return true;
}
}
