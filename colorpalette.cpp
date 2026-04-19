#include "colorpalette.h"
#include "canvaswidget.h"
#include <QGridLayout>
#include <QPushButton>

ColorPalette::ColorPalette(CanvasWidget *canvas, QWidget *parent)
    : QWidget(parent)
    , m_canvas(canvas)
{
    // Создаём сеточный layout для размещения кнопок палитры
    m_layout = new QGridLayout(this);
    setLayout(m_layout);

    // Генерируем палитру начального размера (4x4)
    generatePalette();
}

void ColorPalette::setPaletteSize(int size)
{
    // Проверяем, что размер изменился и находится в допустимом диапазоне
    if (size == m_paletteSize || size < 1 || size > 32)
        return;

    m_paletteSize = size;
    generatePalette();
}

void ColorPalette::generatePalette()
{
    // Удаляем все старые виджеты из layout
    QLayoutItem *child;
    while ((child = m_layout->takeAt(0)) != nullptr) {
        delete child->widget(); // Удаляем сам виджет (кнопку)
        delete child;           // Удаляем элемент layout
    }

    const int btnSize = 30; // Фиксированный размер кнопки в пикселях

    // Вложенные циклы для создания сетки кнопок размером m_paletteSize x m_paletteSize
    for (int row = 0; row < m_paletteSize; ++row) {
        for (int col = 0; col < m_paletteSize; ++col) {
            // Вычисляем цвет для данной позиции в палитре
            QColor color = getColorForIndex(row, col, m_paletteSize);

            // Создаём кнопку
            QPushButton *btn = new QPushButton;
            btn->setFixedSize(btnSize, btnSize);

            // Устанавливаем стиль через таблицу стилей Qt (CSS-подобный синтаксис)
            // color.name() возвращает строку в формате "#RRGGBB"
            btn->setStyleSheet(QString("background-color: %1; border: 1px solid gray;")
                                   .arg(color.name()));

            // Соединяем сигнал clicked() кнопки с лямбда-функцией,
            // которая вызывает onColorSelected с соответствующим цветом
            // Лямбда захватывает [this, color] — указатель на текущий объект и цвет
            connect(btn, &QPushButton::clicked, [this, color]() {
                onColorSelected(color);
            });

            // Добавляем кнопку в layout на позицию (row, col)
            m_layout->addWidget(btn, row, col);
        }
    }
}

QColor ColorPalette::getColorForIndex(int row, int col, int size) const
{
    // Вычисляем линейный индекс от 0 до size*size-1
    int index = row * size + col;

    // Равномерно распределяем оттенок (hue) от 0 до 360 градусов по всей палитре
    double hue = (static_cast<double>(index) / (size * size)) * 360.0;

    // Создаём цвет с насыщенностью 0.8 и яркостью 0.9
    // fromHsvF принимает значения от 0.0 до 1.0, поэтому делим hue на 360
    return QColor::fromHsvF(hue / 360.0, 0.8, 0.9);
}

void ColorPalette::onColorSelected(const QColor &color)
{
    // Если указатель на холст валиден, устанавливаем ему текущий цвет
    if (m_canvas)
        m_canvas->setCurrentColor(color);
}