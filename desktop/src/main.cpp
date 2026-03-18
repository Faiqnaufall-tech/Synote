#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>

#include "db/Database.h"
#include "AppWindow.h"
#include "settings/Settings.h"
#include "settings/LoginDialog.h"

namespace {
bool validateToken(const Settings& s) {
    if (s.baseUrl.isEmpty() || s.token.isEmpty()) {
        return false;
    }
    QNetworkAccessManager manager;
    QNetworkRequest req(QUrl(s.baseUrl + "/auth/me"));
    req.setRawHeader("Authorization", ("Bearer " + s.token).toUtf8());

    QNetworkReply* reply = manager.get(req);
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    reply->deleteLater();
    return status >= 200 && status < 300;
}
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QFile styleFile(":/styles/app.qss");
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        app.setStyleSheet(styleFile.readAll());
    }

    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    const QString dbPath = dataDir + QDir::separator() + "notes.db";

    if (!Database::instance().open(dbPath)) {
        QMessageBox::critical(nullptr, "DB", Database::instance().lastError());
        return 1;
    }

    Settings s = Settings::load();
    if (!validateToken(s)) {
        LoginDialog login;
        if (login.exec() != QDialog::Accepted || !login.loginSuccessful()) {
            return 0;
        }
    }

    AppWindow window;
    window.show();

    return app.exec();
}
