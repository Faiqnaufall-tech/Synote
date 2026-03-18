#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QLabel>

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);

private slots:
    void onSave();
    void onLogout();

private:
    void load();

    QLineEdit* baseUrlEdit_;
    QLineEdit* tokenEdit_;
    QLabel* statusLabel_;
    QPushButton* logoutBtn_;
};
