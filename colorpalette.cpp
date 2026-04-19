#include "colorpalette.h"
#include "canvaswidget.h"
#include <QGridLayout>
#include <QPushButton>

ColorPalette::ColorPalette(CanvasWidget *canvas, QWidget *parent)
    : QWidget(parent), m_canvas(canvas)
{
    m_layout = new QGridLayout(this);
    setLayout(m_layout);
    generatePalette();
}

void ColorPalette::setPaletteSize(int size)
{
    if (size == m_paletteSize || size < 1 || size > 16)
        return;
    m_paletteSize = size;
    generatePalette();
}

void ColorPalette::generatePalette()
{
    // Очищаем layout
    QLayoutItem *child;
    while ((child = m_layout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    const int btnSize = 30;
    for (int row = 0; row < m_paletteSize; ++row) {
        for (int col = 0; col < m_paletteSize; ++col) {
            QColor color = getColorForIndex(row, col, m_paletteSize);
            QPushButton *btn = new QPushButton;
            btn->setFixedSize(btnSize, btnSize);
            btn->setStyleSheet(QString("background-color: %1; border: 1px solid gray;")
                                   .arg(color.name()));
            connect(btn, &QPushButton::clicked, [this, color]() {
                onColorSelected(color);
            });
            m_layout->addWidget(btn, row, col);
        }
    }
}

QColor ColorPalette::getColorForIndex(int row, int col, int size) const
{
    // Равномерное распределение оттенка по всей сетке
    double hue = (static_cast<double>(row * size + col) / (size * size)) * 360.0;
    return QColor::fromHsvF(hue / 360.0, 0.8, 0.9);
}

void ColorPalette::onColorSelected(const QColor &color)
{
    if (m_canvas)
        m_canvas->setCurrentColor(color);
}