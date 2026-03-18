#pragma once

#include <QString>

struct Settings {
    QString baseUrl;
    QString token;

    static Settings load();
    bool save() const;
};
