#pragma once

#include <QString>
#include <QSqlDatabase>

class Database {
public:
    static Database& instance();

    bool open(const QString& path);
    bool isOpen() const;
    QString lastError() const;

private:
    Database() = default;
    bool runMigrations();

    QSqlDatabase db_;
    QString lastError_;
};
