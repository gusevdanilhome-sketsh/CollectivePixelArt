#ifndef CANVASWIDGET_H
#define CANVASWIDGET_H

#include <QWidget>
#include <QColor>
#include <QImage>
#include <QPoint>

class CanvasWidget : public QWidget {
    Q_OBJECT
public:
    enum Tool {
        Brush,
        Eraser,
        Fill,
        Picker,
        Rectangle,
        Ellipse,
        Line,
        Triangle
    };
    Q_ENUM(Tool)

    explicit CanvasWidget(QWidget *parent = nullptr);

    void setCurrentColor(const QColor &color);
    QColor currentColor() const { return m_currentRgb; }

    void setAlpha(int alpha);
    int alpha() const { return m_alpha; }

    void setPixelSize(int size);
    int pixelSize() const { return m_pixelSize; }

    void setCanvasSize(const QSize &size);
    QSize canvasSize() const { return m_canvasSize; }

    void setImage(const QImage &image);
    QImage image() const { return m_image; }

    void setGhostLayer(const QImage &ghost);
    void clearGhostLayer();

    void setTool(Tool tool);
    Tool tool() const { return m_currentTool; }
    void setBrushSize(int size);

signals:
    void currentColorChanged(const QColor &color);
    void canvasChanged();
    void colorPicked(const QColor &color);
    void toolChanged(Tool tool);
    void viewportChanged();
    void pixelSizeChanged(int size);
    void sizeChanged(const QSize &size);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void drawPixel(const QPoint &pixelPos);
    void drawBrush(const QPoint &centerPixel);
    void eraseAt(const QPoint &pixelPos);
    void floodFill(const QPoint &startPixel);
    void pickColorAt(const QPoint &pixelPos);
    void applyShape(const QPoint &start, const QPoint &end);
    QPoint pixelFromPoint(const QPoint &point) const;
    void drawCheckerboard(QPainter &painter);
    void drawShapePreview(QPainter &painter);

    QImage m_image;
    QImage m_ghostImage;
    QSize m_canvasSize;
    int m_pixelSize = 10;
    QColor m_currentRgb = Qt::black;
    int m_alpha = 255;
    bool m_drawing = false;

    Tool m_currentTool = Brush;
    int m_brushSize = 1;
    QPoint m_lastDrawnPixel;
    QPoint m_shapeStart;
    QPoint m_shapeCurrent;
    bool m_shapeActive = false;
};

#endif // CANVASWIDGET_H