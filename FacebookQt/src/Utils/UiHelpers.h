#pragma once

#include <QFrame>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QString>

namespace UiHelpers {
    QFrame* card(QWidget* parent = nullptr);
    QLabel* title(const QString& text, QWidget* parent = nullptr);
    QLabel* muted(const QString& text, QWidget* parent = nullptr);
    QLabel* avatar(const QString& name, int size = 44, QWidget* parent = nullptr);
    QPushButton* button(const QString& text, const QString& objectName = QString(), QWidget* parent = nullptr);
    void clearLayout(QLayout* layout);
}
