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

        void setCurrentColor(const QColor &color) { m_currentColor = color; }
        QColor currentColor() const { return m_currentColor; }

        void setPixelSize(int size);                // размер одного пикселя в пикселях экрана
        void setCanvasSize(int width, int height);  // размер холста в пикселях (логических)

        QSize canvasSize() const { return m_canvasSize; }

    protected:
        void paintEvent(QPaintEvent *event) override;
        void mousePressEvent(QMouseEvent *event) override;
        void mouseMoveEvent(QMouseEvent *event) override;
        void wheelEvent(QWheelEvent *event) override;   // масштабирование

    private:
        void drawPixel(const QPoint &pixelPos);
        QPoint pixelFromPoint(const QPoint &point) const;

        QVector<QVector<QColor>> m_pixels;  // двумерный массив цветов
        QSize m_canvasSize;                 // количество пикселей по ширине и высоте
        int m_pixelSize = 10;               // размер отображения одного пикселя (zoom)
        QColor m_currentColor = Qt::black;
        bool m_drawing = false;
};

#endif // CANVASWIDGET_H