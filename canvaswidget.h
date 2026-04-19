#ifndef CANVASWIDGET_H
#define CANVASWIDGET_H

#include <QWidget>
#include <QColor>
#include <QVector>
#include <QPoint>

class CanvasWidget : public QWidget {
    Q_OBJECT
public:
    explicit CanvasWidget(QWidget *parent = nullptr);

    void setCurrentColor(const QColor &color);
    QColor currentColor() const { return m_currentRgb; }

    void setAlpha(int alpha);
    int alpha() const { return m_alpha; }

    void setPixelSize(int size);
    void setCanvasSize(int width, int height);
    QSize canvasSize() const { return m_canvasSize; }

    // Новый метод: делает все непрозрачные пиксели полупрозрачными (умножает альфу на factor).
    void applyGhostEffect(float factor = 0.5f);

    // Возвращает копию текущего массива пикселей (для сохранения кадра).
    QVector<QVector<QColor>> getPixelsCopy() const;

    // Загружает пиксели из сохранённого массива (для переключения кадров).
    void setPixels(const QVector<QVector<QColor>> &pixels);

signals:
    // Сигнал, испускаемый при изменении текущего цвета (RGB + альфа).
    void currentColorChanged(const QColor &color);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void drawPixel(const QPoint &pixelPos);
    QPoint pixelFromPoint(const QPoint &point) const;
    void drawCheckerboard(QPainter &painter);

    QVector<QVector<QColor>> m_pixels;
    QSize m_canvasSize;
    int m_pixelSize = 10;
    QColor m_currentRgb = Qt::black;
    int m_alpha = 255;
    bool m_drawing = false;
};

#endif // CANVASWIDGET_H