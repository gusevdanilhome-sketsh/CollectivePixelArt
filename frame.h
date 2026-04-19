#ifndef FRAME_H
#define FRAME_H

#include <QList>
#include <QSize>
#include <QImage>
#include "layer.h"

class Frame
{
public:
    Frame(const QSize &size = QSize(32, 32));

    QList<Layer*> layers() const { return m_layers; }
    void addLayer(Layer *layer);
    void insertLayer(int index, Layer *layer);
    void removeLayer(int index);
    void moveLayer(int from, int to);

    QImage compositeImage() const;

    QSize size() const { return m_size; }

private:
    QList<Layer*> m_layers;
    QSize m_size;
};

#endif // FRAME_H