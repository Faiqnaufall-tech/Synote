#include "Keychain.h"

#ifdef HAVE_QTKEYCHAIN
#include <QtKeychain/keychain.h>
#include <QEventLoop>
#endif

namespace Keychain {

bool isAvailable() {
#ifdef HAVE_QTKEYCHAIN
    return true;
#else
    return false;
#endif
}

QString loadOpenAIKey(QString* errorOut) {
#ifdef HAVE_QTKEYCHAIN
    QKeychain::ReadPasswordJob job("Synote");
    job.setKey("openai_api_key");
    QEventLoop loop;
    QObject::connect(&job, &QKeychain::Job::finished, &loop, &QEventLoop::quit);
    job.start();
    loop.exec();

    if (job.error()) {
        if (errorOut) {
            *errorOut = job.errorString();
        }
        return QString();
    }
    return job.textData();
#else
    if (errorOut) {
        *errorOut = "QtKeychain tidak tersedia";
    }
    return QString();
#endif
}

bool storeOpenAIKey(const QString& key, QString* errorOut) {
#ifdef HAVE_QTKEYCHAIN
    QKeychain::WritePasswordJob job("Synote");
    job.setKey("openai_api_key");
    job.setTextData(key);
    QEventLoop loop;
    QObject::connect(&job, &QKeychain::Job::finished, &loop, &QEventLoop::quit);
    job.start();
    loop.exec();

    if (job.error()) {
        if (errorOut) {
            *errorOut = job.errorString();
        }
        return false;
    }
    return true;
#else
    if (errorOut) {
        *errorOut = "QtKeychain tidak tersedia";
    }
    return false;
#endif
}

}
