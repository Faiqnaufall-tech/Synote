#include "SettingsDialog.h"
#include "Settings.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Pengaturan Sync");
    resize(420, 240);

    QVBoxLayout* root = new QVBoxLayout(this);

    baseUrlEdit_ = new QLineEdit();
    baseUrlEdit_->setPlaceholderText("https://your-render-domain");

    tokenEdit_ = new QLineEdit();
    tokenEdit_->setPlaceholderText("Token JWT");
    tokenEdit_->setReadOnly(true);

    statusLabel_ = new QLabel();

    logoutBtn_ = new QPushButton("Logout");
    QPushButton* saveBtn = new QPushButton("Simpan");

    QHBoxLayout* authRow = new QHBoxLayout();
    authRow->addWidget(logoutBtn_);
    authRow->addStretch();
    authRow->addWidget(saveBtn);

    root->addWidget(new QLabel("Base URL"));
    root->addWidget(baseUrlEdit_);
    root->addWidget(new QLabel("Token"));
    root->addWidget(tokenEdit_);
    root->addLayout(authRow);
    root->addWidget(statusLabel_);

    connect(saveBtn, &QPushButton::clicked, this, &SettingsDialog::onSave);
    connect(logoutBtn_, &QPushButton::clicked, this, &SettingsDialog::onLogout);

    load();
}

void SettingsDialog::load() {
    const Settings s = Settings::load();
    baseUrlEdit_->setText(s.baseUrl);
    tokenEdit_->setText(s.token);
}

void SettingsDialog::onSave() {
    Settings s;
    s.baseUrl = baseUrlEdit_->text().trimmed();
    s.token = tokenEdit_->text().trimmed();
    if (!s.save()) {
        statusLabel_->setText("Gagal menyimpan settings");
        return;
    }
    statusLabel_->setText("Settings tersimpan");
}

void SettingsDialog::onLogout() {
    tokenEdit_->clear();
    onSave();
    statusLabel_->setText("Logout berhasil");
}
