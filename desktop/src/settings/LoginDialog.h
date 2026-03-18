#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QLabel>

class LoginDialog : public QDialog {
    Q_OBJECT

public:
    explicit LoginDialog(QWidget* parent = nullptr);
    bool loginSuccessful() const;

private slots:
    void onLogin();
    void onRegister();

private:
    bool authRequest(const QString& path);

    QLineEdit* baseUrlEdit_;
    QLineEdit* emailEdit_;
    QLineEdit* passwordEdit_;
    QLabel* statusLabel_;
    bool ok_;
};
