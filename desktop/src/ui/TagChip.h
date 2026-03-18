#pragma once

#include <QFrame>
#include <QString>

class TagChip : public QFrame {
    Q_OBJECT

public:
    explicit TagChip(const QString& text, QWidget* parent = nullptr);
    QString text() const;

signals:
    void removed(const QString& text);

private:
    QString text_;
};
