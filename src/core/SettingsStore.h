#pragma once
#include "Preferences.h"

namespace pip {
class SettingsStore final {
public:
    explicit SettingsStore(QString path);
    [[nodiscard]] Preferences load() const;
    [[nodiscard]] bool save(const Preferences& preferences, QString* error = nullptr) const;
private:
    QString path_;
};
}
