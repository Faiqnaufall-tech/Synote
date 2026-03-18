#include "TagChip.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>

TagChip::TagChip(const QString& text, QWidget* parent) : QFrame(parent), text_(text) {
    setObjectName("TagChip");
    setStyleSheet("#TagChip { border: 1px solid #C9C9C9; border-radius: 10px; padding: 2px 6px; background: #F5F5F5; }");

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(6, 2, 6, 2);
    layout->setSpacing(4);

    QLabel* label = new QLabel(text, this);
    QToolButton* closeBtn = new QToolButton(this);
    closeBtn->setText("x");
    closeBtn->setAutoRaise(true);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet("QToolButton { border: none; }");

    layout->addWidget(label);
    layout->addWidget(closeBtn);

    connect(closeBtn, &QToolButton::clicked, this, [this]() {
        emit removed(text_);
        deleteLater();
    });
}

QString TagChip::text() const {
    return text_;
}
