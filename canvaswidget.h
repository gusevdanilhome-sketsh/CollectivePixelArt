#ifndef CANVASWIDGET_H
#define CANVASWIDGET_H

#include <QWidget>
#include <QColor>
#include <QVector>
#include <QPoint>

class CanvasWidget : public QWidget {
    Q_OBJECT
public:
    enum Tool {
        Brush,      // Обычная кисть
        Eraser,     // Ластик (рисует прозрачным)
        Fill,       // Заливка области
        Picker      // Пипетка (выбор цвета)
    };
    Q_ENUM(Tool)

    explicit CanvasWidget(QWidget *parent = nullptr);

    void setCurrentColor(const QColor &color);
    QColor currentColor() const { return m_currentRgb; }

    void setAlpha(int alpha);
    int alpha() const { return m_alpha; }

    void setPixelSize(int size);
    void setCanvasSize(int width, int height);
    QSize canvasSize() const { return m_canvasSize; }

    void setGhostLayer(const QVector<QVector<QColor>> &ghostPixels);
    void clearGhostLayer();
    QVector<QVector<QColor>> createGhostFromCurrent(float factor = 0.5f) const;

    QVector<QVector<QColor>> getPixelsCopy() const;
    void setPixels(const QVector<QVector<QColor>> &pixels);

    // Инструменты
    void setTool(Tool tool);
    Tool tool() const { return m_currentTool; }
    void setBrushSize(int size);   // Размер кисти (в пикселях холста)

signals:
    void currentColorChanged(const QColor &color);
    void canvasChanged();
    void colorPicked(const QColor &color); // Сигнал при использовании пипетки

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void drawPixel(const QPoint &pixelPos);
    void drawBrush(const QPoint &centerPixel);
    void eraseAt(const QPoint &pixelPos);
    void floodFill(const QPoint &startPixel);
    void pickColorAt(const QPoint &pixelPos);
    QPoint pixelFromPoint(const QPoint &point) const;
    void drawCheckerboard(QPainter &painter);

    QVector<QVector<QColor>> m_pixels;
    QVector<QVector<QColor>> m_ghostPixels;
    QSize m_canvasSize;
    int m_pixelSize = 10;
    QColor m_currentRgb = Qt::black;
    int m_alpha = 255;
    bool m_drawing = false;

    Tool m_currentTool = Brush;
    int m_brushSize = 1;            // Радиус в пикселях холста (1 = 1x1, 2 = 3x3 и т.д.)
    QPoint m_lastDrawnPixel;        // Последняя отрисованная позиция (для сглаживания линий)
};

#endif // CANVASWIDGET_H