#include "canvaswidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QScrollArea>
#include <QScrollBar>

CanvasWidget::CanvasWidget(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setFixedSize(400, 400);
    setCanvasSize(32, 32);
}

void CanvasWidget::setCurrentColor(const QColor &color)
{
    m_currentRgb = color;   // сохраняем только RGB, альфа отдельно
}

void CanvasWidget::setAlpha(int alpha)
{
    m_alpha = qBound(0, alpha, 255);
}

void CanvasWidget::setPixelSize(int size)
{
    m_pixelSize = size;
    setFixedSize(m_canvasSize.width() * m_pixelSize,
                 m_canvasSize.height() * m_pixelSize);
    update();
}

void CanvasWidget::setCanvasSize(int width, int height)
{
    m_canvasSize = QSize(width, height);
    m_pixels.resize(height);
    for (int i = 0; i < height; ++i) {
        m_pixels[i].resize(width);
        for (int j = 0; j < width; ++j) {
            m_pixels[i][j] = Qt::transparent; // полностью прозрачный
        }
    }
    setFixedSize(width * m_pixelSize, height * m_pixelSize);
    update();
}

void CanvasWidget::drawCheckerboard(QPainter &painter)
{
    const int cellSize = 10; // размер клетки шахматной доски
    for (int y = 0; y < height(); y += cellSize) {
        for (int x = 0; x < width(); x += cellSize) {
            bool white = ((x / cellSize) + (y / cellSize)) % 2 == 0;
            painter.fillRect(x, y, cellSize, cellSize,
                             white ? Qt::lightGray : Qt::gray);
        }
    }
}

void CanvasWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    // 1. Рисуем шахматную доску (фон для прозрачности)
    drawCheckerboard(painter);

    // 2. Рисуем пиксели холста
    for (int y = 0; y < m_canvasSize.height(); ++y) {
        for (int x = 0; x < m_canvasSize.width(); ++x) {
            const QColor &col = m_pixels[y][x];
            if (col.alpha() > 0) {  // рисуем только непрозрачные/полупрозрачные
                painter.fillRect(x * m_pixelSize, y * m_pixelSize,
                                 m_pixelSize, m_pixelSize, col);
            }
        }
    }

    // 3. Рисуем сетку поверх
    painter.setPen(QPen(Qt::black, 1));
    for (int x = 0; x <= m_canvasSize.width(); ++x) {
        painter.drawLine(x * m_pixelSize, 0,
                         x * m_pixelSize, height());
    }
    for (int y = 0; y <= m_canvasSize.height(); ++y) {
        painter.drawLine(0, y * m_pixelSize,
                         width(), y * m_pixelSize);
    }
}

QPoint CanvasWidget::pixelFromPoint(const QPoint &point) const
{
    int x = point.x() / m_pixelSize;
    int y = point.y() / m_pixelSize;
    if (x < 0 || x >= m_canvasSize.width() ||
        y < 0 || y >= m_canvasSize.height())
        return QPoint(-1, -1);
    return QPoint(x, y);
}

void CanvasWidget::drawPixel(const QPoint &pixelPos)
{
    if (pixelPos.x() < 0 || pixelPos.y() < 0)
        return;

    // Создаём цвет с текущими RGB и альфа-каналом
    QColor color(m_currentRgb.red(), m_currentRgb.green(), m_currentRgb.blue(), m_alpha);
    m_pixels[pixelPos.y()][pixelPos.x()] = color;
    update();
}

void CanvasWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_drawing = true;
        drawPixel(pixelFromPoint(event->pos()));
    }
}

void CanvasWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_drawing && (event->buttons() & Qt::LeftButton)) {
        drawPixel(pixelFromPoint(event->pos()));
    }
}

void CanvasWidget::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        int delta = event->angleDelta().y();
        int newSize = m_pixelSize + (delta > 0 ? 1 : -1);
        newSize = qBound(4, newSize, 64);
        if (newSize != m_pixelSize) {
            setPixelSize(newSize);
            if (parentWidget()) {
                QScrollArea *scrollArea = qobject_cast<QScrollArea*>(parentWidget());
                if (scrollArea)
                    scrollArea->updateGeometry();
            }
        }
        event->accept();
    } else {
        event->ignore();
    }
}