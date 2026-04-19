#include "canvaswidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QQueue>
#include <QSet>

CanvasWidget::CanvasWidget(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setFixedSize(400, 400);
    setCanvasSize(32, 32);
}

void CanvasWidget::setCurrentColor(const QColor &color)
{
    m_currentRgb = color;
    emit currentColorChanged(QColor(m_currentRgb.red(), m_currentRgb.green(), m_currentRgb.blue(), m_alpha));
}

void CanvasWidget::setAlpha(int alpha)
{
    m_alpha = qBound(0, alpha, 255);
    emit currentColorChanged(QColor(m_currentRgb.red(), m_currentRgb.green(), m_currentRgb.blue(), m_alpha));
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
            m_pixels[i][j] = Qt::transparent;
        }
    }
    m_ghostPixels.clear();
    setFixedSize(width * m_pixelSize, height * m_pixelSize);
    update();
    emit canvasChanged();
}

void CanvasWidget::setGhostLayer(const QVector<QVector<QColor>> &ghostPixels)
{
    if (ghostPixels.size() == m_canvasSize.height() &&
        !ghostPixels.isEmpty() && ghostPixels[0].size() == m_canvasSize.width()) {
        m_ghostPixels = ghostPixels;
    } else {
        m_ghostPixels.clear();
    }
    update();
}

void CanvasWidget::clearGhostLayer()
{
    m_ghostPixels.clear();
    update();
}

QVector<QVector<QColor>> CanvasWidget::createGhostFromCurrent(float factor) const
{
    QVector<QVector<QColor>> ghost = m_pixels;
    for (int y = 0; y < ghost.size(); ++y) {
        for (int x = 0; x < ghost[y].size(); ++x) {
            QColor &col = ghost[y][x];
            if (col.alpha() > 0) {
                int newAlpha = qBound(0, static_cast<int>(col.alpha() * factor), 255);
                col.setAlpha(newAlpha);
            }
        }
    }
    return ghost;
}

QVector<QVector<QColor>> CanvasWidget::getPixelsCopy() const
{
    return m_pixels;
}

void CanvasWidget::setPixels(const QVector<QVector<QColor>> &pixels)
{
    if (pixels.size() == m_canvasSize.height() && !pixels.isEmpty() && pixels[0].size() == m_canvasSize.width()) {
        m_pixels = pixels;
        update();
        emit canvasChanged();
    }
}

void CanvasWidget::setTool(Tool tool)
{
    m_currentTool = tool;
    // При смене инструмента сбрасываем флаг рисования
    m_drawing = false;
}

void CanvasWidget::setBrushSize(int size)
{
    m_brushSize = qBound(1, size, 10);
}

void CanvasWidget::drawPixel(const QPoint &pixelPos)
{
    if (pixelPos.x() < 0 || pixelPos.x() >= m_canvasSize.width() ||
        pixelPos.y() < 0 || pixelPos.y() >= m_canvasSize.height())
        return;

    QColor color(m_currentRgb.red(), m_currentRgb.green(), m_currentRgb.blue(), m_alpha);
    m_pixels[pixelPos.y()][pixelPos.x()] = color;
    update();
    emit canvasChanged();
}

void CanvasWidget::drawBrush(const QPoint &centerPixel)
{
    int radius = m_brushSize - 1;
    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            // Квадратная кисть (можно заменить на круглую)
            QPoint pt(centerPixel.x() + dx, centerPixel.y() + dy);
            if (pt.x() >= 0 && pt.x() < m_canvasSize.width() &&
                pt.y() >= 0 && pt.y() < m_canvasSize.height()) {
                QColor color(m_currentRgb.red(), m_currentRgb.green(), m_currentRgb.blue(), m_alpha);
                m_pixels[pt.y()][pt.x()] = color;
            }
        }
    }
    update();
    emit canvasChanged();
}

void CanvasWidget::eraseAt(const QPoint &pixelPos)
{
    if (pixelPos.x() < 0 || pixelPos.x() >= m_canvasSize.width() ||
        pixelPos.y() < 0 || pixelPos.y() >= m_canvasSize.height())
        return;

    m_pixels[pixelPos.y()][pixelPos.x()] = Qt::transparent;
    update();
    emit canvasChanged();
}

void CanvasWidget::floodFill(const QPoint &startPixel)
{
    if (startPixel.x() < 0 || startPixel.x() >= m_canvasSize.width() ||
        startPixel.y() < 0 || startPixel.y() >= m_canvasSize.height())
        return;

    QColor targetColor = m_pixels[startPixel.y()][startPixel.x()];
    QColor fillColor(m_currentRgb.red(), m_currentRgb.green(), m_currentRgb.blue(), m_alpha);

    if (targetColor == fillColor)
        return; // Уже залито этим цветом

    QQueue<QPoint> queue;
    QSet<QPoint> visited;
    queue.enqueue(startPixel);
    visited.insert(startPixel);

    while (!queue.isEmpty()) {
        QPoint pt = queue.dequeue();
        m_pixels[pt.y()][pt.x()] = fillColor;

        // Соседи (4-связность)
        QPoint neighbors[4] = {
            QPoint(pt.x() + 1, pt.y()),
            QPoint(pt.x() - 1, pt.y()),
            QPoint(pt.x(), pt.y() + 1),
            QPoint(pt.x(), pt.y() - 1)
        };
        for (const QPoint &n : neighbors) {
            if (n.x() >= 0 && n.x() < m_canvasSize.width() &&
                n.y() >= 0 && n.y() < m_canvasSize.height() &&
                !visited.contains(n) &&
                m_pixels[n.y()][n.x()] == targetColor) {
                queue.enqueue(n);
                visited.insert(n);
            }
        }
    }

    update();
    emit canvasChanged();
}

void CanvasWidget::pickColorAt(const QPoint &pixelPos)
{
    if (pixelPos.x() < 0 || pixelPos.x() >= m_canvasSize.width() ||
        pixelPos.y() < 0 || pixelPos.y() >= m_canvasSize.height())
        return;

    QColor picked = m_pixels[pixelPos.y()][pixelPos.x()];
    if (picked.alpha() == 0) {
        // Если пиксель прозрачный, можно выбрать белый или не менять. Выберем белый.
        picked = Qt::white;
    }
    // Устанавливаем как текущий цвет (альфа остаётся прежней)
    m_currentRgb = picked;
    setAlpha(picked.alpha());
    emit colorPicked(picked);
    emit currentColorChanged(QColor(m_currentRgb.red(), m_currentRgb.green(), m_currentRgb.blue(), m_alpha));
}

void CanvasWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    drawCheckerboard(painter);

    // Фоновый слой
    if (!m_ghostPixels.isEmpty()) {
        for (int y = 0; y < m_canvasSize.height(); ++y) {
            for (int x = 0; x < m_canvasSize.width(); ++x) {
                const QColor &col = m_ghostPixels[y][x];
                if (col.alpha() > 0) {
                    painter.fillRect(x * m_pixelSize, y * m_pixelSize,
                                     m_pixelSize, m_pixelSize, col);
                }
            }
        }
    }

    // Основной слой
    for (int y = 0; y < m_canvasSize.height(); ++y) {
        for (int x = 0; x < m_canvasSize.width(); ++x) {
            const QColor &col = m_pixels[y][x];
            if (col.alpha() > 0) {
                painter.fillRect(x * m_pixelSize, y * m_pixelSize,
                                 m_pixelSize, m_pixelSize, col);
            }
        }
    }

    // Сетка
    painter.setPen(QPen(Qt::black, 1));
    for (int x = 0; x <= m_canvasSize.width(); ++x) {
        painter.drawLine(x * m_pixelSize, 0, x * m_pixelSize, height());
    }
    for (int y = 0; y <= m_canvasSize.height(); ++y) {
        painter.drawLine(0, y * m_pixelSize, width(), y * m_pixelSize);
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

void CanvasWidget::drawCheckerboard(QPainter &painter)
{
    const int cellSize = 10;
    for (int y = 0; y < height(); y += cellSize) {
        for (int x = 0; x < width(); x += cellSize) {
            bool white = ((x / cellSize) + (y / cellSize)) % 2 == 0;
            painter.fillRect(x, y, cellSize, cellSize,
                             white ? Qt::lightGray : Qt::gray);
        }
    }
}

void CanvasWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPoint pixel = pixelFromPoint(event->pos());
        if (pixel.x() < 0) return;

        m_drawing = true;
        m_lastDrawnPixel = pixel;

        switch (m_currentTool) {
        case Brush:
            if (m_brushSize == 1)
                drawPixel(pixel);
            else
                drawBrush(pixel);
            break;
        case Eraser:
            eraseAt(pixel);
            break;
        case Fill:
            floodFill(pixel);
            m_drawing = false; // заливка однократная
            break;
        case Picker:
            pickColorAt(pixel);
            m_drawing = false;
            break;
        }
    }
}

void CanvasWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_drawing || !(event->buttons() & Qt::LeftButton))
        return;

    QPoint pixel = pixelFromPoint(event->pos());
    if (pixel.x() < 0) return;

    // Для кисти и ластика рисуем линию между последней точкой и текущей (сглаживание)
    if (m_currentTool == Brush || m_currentTool == Eraser) {
        // Алгоритм Брезенхэма для рисования линии между двумя пикселями
        int x0 = m_lastDrawnPixel.x();
        int y0 = m_lastDrawnPixel.y();
        int x1 = pixel.x();
        int y1 = pixel.y();

        int dx = abs(x1 - x0);
        int dy = -abs(y1 - y0);
        int sx = x0 < x1 ? 1 : -1;
        int sy = y0 < y1 ? 1 : -1;
        int err = dx + dy;

        while (true) {
            if (m_currentTool == Brush) {
                if (m_brushSize == 1)
                    drawPixel(QPoint(x0, y0));
                else
                    drawBrush(QPoint(x0, y0));
            } else {
                eraseAt(QPoint(x0, y0));
            }

            if (x0 == x1 && y0 == y1) break;
            int e2 = 2 * err;
            if (e2 >= dy) {
                err += dy;
                x0 += sx;
            }
            if (e2 <= dx) {
                err += dx;
                y0 += sy;
            }
        }
        m_lastDrawnPixel = pixel;
    }
}

void CanvasWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_drawing = false;
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