#include "Settings.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

namespace {
QString settingsPath() {
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + QDir::separator() + "settings.json";
}
}

Settings Settings::load() {
    Settings s;
    QFile file(settingsPath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return s;
    }
    const QByteArray data = file.readAll();
    const QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        return s;
    }
    const QJsonObject obj = doc.object();
    s.baseUrl = obj.value("base_url").toString();
    s.token = obj.value("token").toString();
    return s;
}

bool Settings::save() const {
    QJsonObject obj;
    obj["base_url"] = baseUrl;
    obj["token"] = token;

    QFile file(settingsPath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    file.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
    return true;
}
