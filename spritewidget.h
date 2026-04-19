#ifndef SPRITEWIDGET_H
#define SPRITEWIDGET_H

#include <QWidget>
#include <QImage>
#include <QMouseEvent>
#include <QWheelEvent>

class SpriteWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SpriteWidget(QWidget *parent = nullptr);

    void setFrame(const QImage &image);
    void clear();
    void setPixelSize(int size);
    int pixelSize() const { return m_pixelSize; }

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void drawCheckerboard(QPainter &painter, const QRect &rect);

    QImage m_image;
    int m_pixelSize = 10;
};

#endif // SPRITEWIDGET_H