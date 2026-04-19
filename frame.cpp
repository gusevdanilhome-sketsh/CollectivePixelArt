#include "frame.h"
#include "layer.h"
#include <QPainter>

Frame::Frame(const QSize &size)
    : m_size(size)
{
}

void Frame::addLayer(Layer *layer)
{
    if (layer) {
        m_layers.append(layer);
    }
}

void Frame::insertLayer(int index, Layer *layer)
{
    if (layer && index >= 0 && index <= m_layers.size()) {
        m_layers.insert(index, layer);
    }
}

void Frame::removeLayer(int index)
{
    if (index >= 0 && index < m_layers.size()) {
        delete m_layers.takeAt(index);
    }
}

void Frame::moveLayer(int from, int to)
{
    if (from >= 0 && from < m_layers.size() &&
        to >= 0 && to < m_layers.size() && from != to) {
        m_layers.move(from, to);
    }
}

QImage Frame::compositeImage() const
{
    if (m_layers.isEmpty()) {
        return QImage(m_size, QImage::Format_ARGB32);
    }

    // Создаём результирующее изображение
    QImage result(m_size, QImage::Format_ARGB32);
    result.fill(Qt::transparent);

    QPainter painter(&result);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    // Рисуем слои снизу вверх (первый в списке - нижний слой)
    for (Layer *layer : m_layers) {
        if (!layer->isVisible()) {
            continue; // Пропускаем невидимые слои
        }

        QImage layerImage = layer->image();
        if (layerImage.isNull()) {
            continue;
        }

        // Применяем глобальную прозрачность слоя
        int opacity = layer->opacity();
        if (opacity < 255) {
            painter.setOpacity(opacity / 255.0);
        } else {
            painter.setOpacity(1.0);
        }

        // Рисуем слой на результирующем изображении
        painter.drawImage(0, 0, layerImage);
    }

    painter.end();
    return result;
}