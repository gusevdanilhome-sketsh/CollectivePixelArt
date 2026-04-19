#include "colorpalette.h"
#include "canvaswidget.h"
#include <QGridLayout>
#include <QPushButton>
#include <QColorDialog>

ColorPalette::ColorPalette(CanvasWidget *canvas, QWidget *parent): QWidget(parent), m_canvas(canvas) {
    QGridLayout *layout = new QGridLayout(this);

    // Набор базовых цветов
    QList<QColor> colors = {
        Qt::black, Qt::white, Qt::red, Qt::green, Qt::blue,
        Qt::cyan, Qt::magenta, Qt::yellow, Qt::gray, Qt::darkRed,
        Qt::darkGreen, Qt::darkBlue, Qt::darkCyan, Qt::darkMagenta,
        Qt::darkYellow, Qt::darkGray
    };

    int row = 0, col = 0;
    for (const QColor &color : colors) {
        QPushButton *btn = new QPushButton;
        btn->setFixedSize(30, 30);
        btn->setStyleSheet(QString("background-color: %1; border: 1px solid gray;").arg(color.name()));
        connect(btn, &QPushButton::clicked, [this, color]() {onColorSelected(color);});
        layout->addWidget(btn, row, col);
        col++;
        if (col >= 8) { col = 0; row++; }
    }

    // Кнопка "Другой цвет..." для выбора из диалога
    QPushButton *customBtn = new QPushButton("...");
    customBtn->setFixedSize(30, 30);
    connect(customBtn, &QPushButton::clicked, [this]() {QColor color = QColorDialog::getColor(m_canvas->currentColor(), this); if (color.isValid()) onColorSelected(color);});
    layout->addWidget(customBtn, row, col);

    setLayout(layout);
}

void ColorPalette::onColorSelected(const QColor &color) {
    if (m_canvas)
        m_canvas->setCurrentColor(color);
}