#ifndef LAYER_H
#define LAYER_H

#include <QImage>
#include <QString>

class Layer
{
public:
    Layer(const QString &name = QString(), const QSize &size = QSize(32, 32));

    QImage image() const { return m_image; }
    void setImage(const QImage &img);

    QString name() const { return m_name; }
    void setName(const QString &name) { m_name = name; }

    bool isVisible() const { return m_visible; }
    void setVisible(bool visible) { m_visible = visible; }

    int opacity() const { return m_opacity; }
    void setOpacity(int opacity);

    void setPixel(int x, int y, const QColor &color);
    QColor pixel(int x, int y) const;

    QSize size() const { return m_image.size(); }
    void resize(const QSize &size);

private:
    QImage m_image;
    QString m_name;
    bool m_visible = true;
    int m_opacity = 255;
};

#endif // LAYER_H