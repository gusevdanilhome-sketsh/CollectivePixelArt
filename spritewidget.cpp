#include "spritewidget.h"
#include <QPainter>

SpriteWidget::SpriteWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(100, 100);
}

void SpriteWidget::setFrame(const QVector<QVector<QColor>> &pixels)
{
    if (pixels.isEmpty()) {
        clear();
        return;
    }

    int height = pixels.size();
    int width = pixels[0].size();
    m_frameSize = QSize(width, height);

    m_image = QImage(width, height, QImage::Format_ARGB32);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            m_image.setPixelColor(x, y, pixels[y][x]);
        }
    }

    update(); // Запрашиваем перерисовку
}

void SpriteWidget::clear()
{
    m_image = QImage();
    m_frameSize = QSize();
    update();
}

void SpriteWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    if (m_image.isNull()) {
        painter.fillRect(rect(), Qt::lightGray);
        painter.drawText(rect(), Qt::AlignCenter, tr("Нет кадра"));
        return;
    }

    // Масштабируем изображение с сохранением пропорций и центрированием
    QSize scaledSize = m_frameSize * m_pixelSize;
    QRect targetRect(0, 0, scaledSize.width(), scaledSize.height());
    targetRect.moveCenter(rect().center());

    painter.drawImage(targetRect, m_image);
}