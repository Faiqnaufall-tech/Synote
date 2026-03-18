#include "Database.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QTextStream>

Database& Database::instance() {
    static Database instance;
    return instance;
}

bool Database::open(const QString& path) {
    if (db_.isOpen()) {
        return true;
    }

    db_ = QSqlDatabase::addDatabase("QSQLITE");
    db_.setDatabaseName(path);
    if (!db_.open()) {
        lastError_ = db_.lastError().text();
        return false;
    }

    return runMigrations();
}

bool Database::isOpen() const {
    return db_.isOpen();
}

QString Database::lastError() const {
    return lastError_;
}

bool Database::runMigrations() {
    QFile file(":/src/db/schema.sql");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        lastError_ = "Gagal membaca schema.sql";
        return false;
    }

    QTextStream in(&file);
    QString sql = in.readAll();
    file.close();

    QSqlQuery query(db_);
    QString current;
    bool inTrigger = false;

    auto flush = [&]() -> bool {
        const QString stmt = current.trimmed();
        current.clear();
        if (stmt.isEmpty()) {
            return true;
        }
        if (!query.exec(stmt)) {
            lastError_ = query.lastError().text();
            return false;
        }
        return true;
    };

    for (int i = 0; i < sql.size(); ++i) {
        const QChar c = sql.at(i);
        current.append(c);

        if (!inTrigger) {
            const QString tail = current.right(30).toUpper();
            if (tail.contains("CREATE TRIGGER")) {
                inTrigger = true;
            }
        }

        if (c == ';') {
            if (!inTrigger) {
                if (!flush()) {
                    return false;
                }
            } else {
                const QString tail = current.right(10).toUpper();
                if (tail.contains("END;")) {
                    inTrigger = false;
                    if (!flush()) {
                        return false;
                    }
                }
            }
        }
    }

    if (!flush()) {
        return false;
    }

    return true;
}
