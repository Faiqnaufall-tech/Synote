#pragma once

#include <QObject>
#include <QString>

class SyncClient : public QObject {
    Q_OBJECT

public:
    explicit SyncClient(QObject* parent = nullptr);

    bool syncNow(QString* errorOut = nullptr);

private:
    bool ensureDeviceId(QString* deviceIdOut, QString* errorOut);
    QString getLastSync();
    bool setLastSync(const QString& isoTime, QString* errorOut);
};
