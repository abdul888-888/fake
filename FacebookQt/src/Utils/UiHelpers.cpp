#include "UiHelpers.h"

#include <QColor>
#include <QGraphicsDropShadowEffect>
#include <QtGlobal>

namespace UiHelpers {

QFrame* card(QWidget* parent) {
    auto* frame = new QFrame(parent);
    frame->setProperty("class", "Card");
    auto* shadow = new QGraphicsDropShadowEffect(frame);
    shadow->setBlurRadius(18);
    shadow->setOffset(0, 4);
    shadow->setColor(QColor(15, 23, 42, 24));
    frame->setGraphicsEffect(shadow);
    return frame;
}

QLabel* title(const QString& text, QWidget* parent) {
    auto* label = new QLabel(text, parent);
    label->setObjectName("PageTitle");
    label->setWordWrap(true);
    return label;
}

QLabel* muted(const QString& text, QWidget* parent) {
    auto* label = new QLabel(text, parent);
    label->setObjectName("Muted");
    label->setWordWrap(true);
    return label;
}

QLabel* avatar(const QString& name, int size, QWidget* parent) {
    auto* label = new QLabel(parent);
    label->setFixedSize(size, size);
    label->setAlignment(Qt::AlignCenter);
    const QString initials = name.split(' ', Qt::SkipEmptyParts)
        .mid(0, 2)
        .join("")
        .left(2)
        .toUpper();
    label->setText(initials.isEmpty() ? "U" : initials);
    label->setStyleSheet(QString(
        "background:#e7f3ff;color:#1877f2;border-radius:%1px;font-weight:800;font-size:%2px;")
        .arg(size / 2)
        .arg(qMax(13, size / 3)));
    return label;
}

QPushButton* button(const QString& text, const QString& objectName, QWidget* parent) {
    auto* pushButton = new QPushButton(text, parent);
    if (!objectName.isEmpty()) pushButton->setObjectName(objectName);
    pushButton->setCursor(Qt::PointingHandCursor);
    return pushButton;
}

void clearLayout(QLayout* layout) {
    if (!layout) return;
    while (QLayoutItem* item = layout->takeAt(0)) {
        if (QWidget* widget = item->widget()) widget->deleteLater();
        if (QLayout* child = item->layout()) clearLayout(child);
        delete item;
    }
}

}
