#pragma once

#include <QString>

namespace Keychain {

bool isAvailable();
QString loadOpenAIKey(QString* errorOut = nullptr);
bool storeOpenAIKey(const QString& key, QString* errorOut = nullptr);

}
