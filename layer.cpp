#include "layer.h"
#include <QPainter>

Layer::Layer(const QString &name, const QSize &size)
    : m_name(name)
{
    // Создаём изображение с альфа-каналом
    m_image = QImage(size, QImage::Format_ARGB32);
    // Заполняем прозрачным цветом
    m_image.fill(Qt::transparent);
}

void Layer::setImage(const QImage &img)
{
    // Убеждаемся, что изображение в правильном формате (с альфа-каналом)
    if (img.format() != QImage::Format_ARGB32) {
        m_image = img.convertToFormat(QImage::Format_ARGB32);
    } else {
        m_image = img;
    }
}

void Layer::setOpacity(int opacity)
{
    // Ограничиваем значение в диапазоне от 0 до 255
    m_opacity = qBound(0, opacity, 255);
}

void Layer::setPixel(int x, int y, const QColor &color)
{
    // Проверяем, что координаты находятся в пределах изображения
    if (x >= 0 && x < m_image.width() && y >= 0 && y < m_image.height()) {
        m_image.setPixelColor(x, y, color);
    }
}

QColor Layer::pixel(int x, int y) const
{
    // Проверяем, что координаты находятся в пределах изображения
    if (x >= 0 && x < m_image.width() && y >= 0 && y < m_image.height()) {
        return m_image.pixelColor(x, y);
    }
    // Возвращаем прозрачный цвет, если координаты вне изображения
    return QColor();
}

void Layer::resize(const QSize &newSize)
{
    // Если размер не изменился, ничего не делаем
    if (m_image.size() == newSize)
        return;

    // Создаём новое изображение нужного размера
    QImage newImage(newSize, QImage::Format_ARGB32);
    newImage.fill(Qt::transparent);

    // Копируем содержимое старого изображения в новое
    QPainter painter(&newImage);
    painter.drawImage(0, 0, m_image);
    painter.end();

    // Заменяем изображение
    m_image = newImage;
}