#include "LoginDialog.h"
#include "Settings.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>

LoginDialog::LoginDialog(QWidget* parent) : QDialog(parent), ok_(false) {
    setWindowTitle("Login");
    resize(420, 240);

    QVBoxLayout* root = new QVBoxLayout(this);

    baseUrlEdit_ = new QLineEdit();
    baseUrlEdit_->setPlaceholderText("https://your-render-domain");

    emailEdit_ = new QLineEdit();
    emailEdit_->setPlaceholderText("Email");

    passwordEdit_ = new QLineEdit();
    passwordEdit_->setPlaceholderText("Password");
    passwordEdit_->setEchoMode(QLineEdit::Password);

    statusLabel_ = new QLabel();

    QPushButton* loginBtn = new QPushButton("Login");
    QPushButton* registerBtn = new QPushButton("Register");
    QPushButton* cancelBtn = new QPushButton("Batal");

    QHBoxLayout* row = new QHBoxLayout();
    row->addWidget(loginBtn);
    row->addWidget(registerBtn);
    row->addStretch();
    row->addWidget(cancelBtn);

    root->addWidget(new QLabel("Base URL"));
    root->addWidget(baseUrlEdit_);
    root->addWidget(new QLabel("Email"));
    root->addWidget(emailEdit_);
    root->addWidget(new QLabel("Password"));
    root->addWidget(passwordEdit_);
    root->addLayout(row);
    root->addWidget(statusLabel_);

    connect(loginBtn, &QPushButton::clicked, this, &LoginDialog::onLogin);
    connect(registerBtn, &QPushButton::clicked, this, &LoginDialog::onRegister);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    const Settings s = Settings::load();
    baseUrlEdit_->setText(s.baseUrl);
}

bool LoginDialog::loginSuccessful() const {
    return ok_;
}

bool LoginDialog::authRequest(const QString& path) {
    const QString baseUrl = baseUrlEdit_->text().trimmed();
    if (baseUrl.isEmpty()) {
        statusLabel_->setText("Base URL belum diisi");
        return false;
    }

    QJsonObject payload;
    payload["email"] = emailEdit_->text().trimmed();
    payload["password"] = passwordEdit_->text();

    QNetworkAccessManager manager;
    QNetworkRequest req(QUrl(baseUrl + path));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = manager.post(req, QJsonDocument(payload).toJson());
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray data = reply->readAll();
    reply->deleteLater();

    if (status < 200 || status >= 300) {
        statusLabel_->setText("Login/register gagal");
        return false;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        statusLabel_->setText("Response tidak valid");
        return false;
    }
    const QJsonObject obj = doc.object();
    const QString token = obj.value("token").toString();
    if (token.isEmpty()) {
        statusLabel_->setText("Login/register gagal");
        return false;
    }

    Settings s;
    s.baseUrl = baseUrl;
    s.token = token;
    s.save();

    ok_ = true;
    accept();
    return true;
}

void LoginDialog::onLogin() {
    authRequest("/auth/login");
}

void LoginDialog::onRegister() {
    authRequest("/auth/register");
}
