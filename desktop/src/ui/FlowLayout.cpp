#include "FlowLayout.h"

#include <QtMath>
#include <QWidget>

FlowLayout::FlowLayout(QWidget* parent, int margin, int hSpacing, int vSpacing)
    : QLayout(parent), m_hSpace(hSpacing), m_vSpace(vSpacing) {
    setContentsMargins(margin, margin, margin, margin);
}

FlowLayout::~FlowLayout() {
    QLayoutItem* item;
    while ((item = takeAt(0))) {
        delete item;
    }
}

void FlowLayout::addItem(QLayoutItem* item) {
    itemList.append(item);
}

int FlowLayout::horizontalSpacing() const {
    if (m_hSpace >= 0) {
        return m_hSpace;
    }
    return smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
}

int FlowLayout::verticalSpacing() const {
    if (m_vSpace >= 0) {
        return m_vSpace;
    }
    return smartSpacing(QStyle::PM_LayoutVerticalSpacing);
}

Qt::Orientations FlowLayout::expandingDirections() const {
    return {};
}

bool FlowLayout::hasHeightForWidth() const {
    return true;
}

int FlowLayout::heightForWidth(int width) const {
    return doLayout(QRect(0, 0, width, 0), true);
}

int FlowLayout::count() const {
    return itemList.size();
}

QLayoutItem* FlowLayout::itemAt(int index) const {
    return itemList.value(index);
}

QLayoutItem* FlowLayout::takeAt(int index) {
    if (index >= 0 && index < itemList.size()) {
        return itemList.takeAt(index);
    }
    return nullptr;
}

QSize FlowLayout::minimumSize() const {
    QSize size;
    for (QLayoutItem* item : itemList) {
        size = size.expandedTo(item->minimumSize());
    }
    const QMargins margins = contentsMargins();
    size += QSize(margins.left() + margins.right(), margins.top() + margins.bottom());
    return size;
}

void FlowLayout::setGeometry(const QRect& rect) {
    QLayout::setGeometry(rect);
    doLayout(rect, false);
}

QSize FlowLayout::sizeHint() const {
    return minimumSize();
}

int FlowLayout::doLayout(const QRect& rect, bool testOnly) const {
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
    int x = effectiveRect.x();
    int y = effectiveRect.y();
    int lineHeight = 0;

    const int spaceX = horizontalSpacing();
    const int spaceY = verticalSpacing();

    for (QLayoutItem* item : itemList) {
        QWidget* wid = item->widget();
        const int space = wid->style()->layoutSpacing(
            QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal);
        const int spaceXUse = spaceX >= 0 ? spaceX : space;
        const int spaceYUse = spaceY >= 0 ? spaceY : space;

        int nextX = x + item->sizeHint().width() + spaceXUse;
        if (nextX - spaceXUse > effectiveRect.right() && lineHeight > 0) {
            x = effectiveRect.x();
            y = y + lineHeight + spaceYUse;
            nextX = x + item->sizeHint().width() + spaceXUse;
            lineHeight = 0;
        }

        if (!testOnly) {
            item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));
        }

        x = nextX;
        lineHeight = qMax(lineHeight, item->sizeHint().height());
    }

    return y + lineHeight - rect.y() + bottom;
}

int FlowLayout::smartSpacing(QStyle::PixelMetric pm) const {
    QObject* parent = this->parent();
    if (!parent) {
        return -1;
    } else if (parent->isWidgetType()) {
        QWidget* pw = static_cast<QWidget*>(parent);
        return pw->style()->pixelMetric(pm, nullptr, pw);
    }
    return static_cast<QLayout*>(parent)->spacing();
}
