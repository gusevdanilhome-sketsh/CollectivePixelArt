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
    setCanvasSize(QSize(32, 32));
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
    emit pixelSizeChanged(m_pixelSize);
}

void CanvasWidget::setCanvasSize(const QSize &size)
{
    m_canvasSize = size;
    m_image = QImage(size, QImage::Format_ARGB32);
    m_image.fill(Qt::transparent);
    m_ghostImage = QImage();
    setFixedSize(size.width() * m_pixelSize, size.height() * m_pixelSize);
    update();
    emit canvasChanged();
}

void CanvasWidget::setImage(const QImage &image)
{
    if (image.size() != m_canvasSize) {
        m_canvasSize = image.size();
        setFixedSize(m_canvasSize.width() * m_pixelSize, m_canvasSize.height() * m_pixelSize);
    }
    m_image = image.convertToFormat(QImage::Format_ARGB32);
    update();
    emit canvasChanged();
}

void CanvasWidget::setGhostLayer(const QImage &ghost)
{
    if (ghost.size() == m_canvasSize) {
        m_ghostImage = ghost;
    } else {
        m_ghostImage = QImage();
    }
    update();
}

void CanvasWidget::clearGhostLayer()
{
    m_ghostImage = QImage();
    update();
}

void CanvasWidget::setTool(Tool tool)
{
    m_currentTool = tool;
    m_drawing = false;
    m_shapeActive = false;
    emit toolChanged(tool);
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
    m_image.setPixelColor(pixelPos, color);
    update();
    emit canvasChanged();
}

void CanvasWidget::drawBrush(const QPoint &centerPixel)
{
    int radius = m_brushSize - 1;
    QColor color(m_currentRgb.red(), m_currentRgb.green(), m_currentRgb.blue(), m_alpha);
    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            QPoint pt(centerPixel.x() + dx, centerPixel.y() + dy);
            if (pt.x() >= 0 && pt.x() < m_canvasSize.width() &&
                pt.y() >= 0 && pt.y() < m_canvasSize.height()) {
                m_image.setPixelColor(pt, color);
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

    m_image.setPixelColor(pixelPos, Qt::transparent);
    update();
    emit canvasChanged();
}

void CanvasWidget::floodFill(const QPoint &startPixel)
{
    if (startPixel.x() < 0 || startPixel.x() >= m_canvasSize.width() ||
        startPixel.y() < 0 || startPixel.y() >= m_canvasSize.height())
        return;

    QColor targetColor = m_image.pixelColor(startPixel);
    QColor fillColor(m_currentRgb.red(), m_currentRgb.green(), m_currentRgb.blue(), m_alpha);

    if (targetColor == fillColor)
        return;

    QQueue<QPoint> queue;
    QSet<QPoint> visited;
    queue.enqueue(startPixel);
    visited.insert(startPixel);

    while (!queue.isEmpty()) {
        QPoint pt = queue.dequeue();
        m_image.setPixelColor(pt, fillColor);

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
                m_image.pixelColor(n) == targetColor) {
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

    QColor picked = m_image.pixelColor(pixelPos);
    if (picked.alpha() == 0) {
        picked = Qt::white;
    }
    m_currentRgb = picked;
    setAlpha(picked.alpha());
    emit colorPicked(picked);
    emit currentColorChanged(QColor(m_currentRgb.red(), m_currentRgb.green(), m_currentRgb.blue(), m_alpha));
}

void CanvasWidget::applyShape(const QPoint &start, const QPoint &end)
{
    if (start.x() < 0 || start.y() < 0 || end.x() < 0 || end.y() < 0)
        return;

    int x0 = start.x(), y0 = start.y();
    int x1 = end.x(), y1 = end.y();
    QColor color(m_currentRgb.red(), m_currentRgb.green(), m_currentRgb.blue(), m_alpha);

    auto setPixel = [this](int x, int y, const QColor &c) {
        if (x >= 0 && x < m_canvasSize.width() && y >= 0 && y < m_canvasSize.height())
            m_image.setPixelColor(x, y, c);
    };

    if (m_currentTool == Rectangle) {
        int xmin = qMin(x0, x1), xmax = qMax(x0, x1);
        int ymin = qMin(y0, y1), ymax = qMax(y0, y1);
        for (int x = xmin; x <= xmax; ++x) {
            setPixel(x, ymin, color);
            setPixel(x, ymax, color);
        }
        for (int y = ymin; y <= ymax; ++y) {
            setPixel(xmin, y, color);
            setPixel(xmax, y, color);
        }
    } else if (m_currentTool == Ellipse) {
        int cx = (x0 + x1) / 2;
        int cy = (y0 + y1) / 2;
        int rx = qAbs(x1 - x0) / 2;
        int ry = qAbs(y1 - y0) / 2;
        if (rx == 0 || ry == 0) return;

        int x = 0, y = ry;
        int rx2 = rx * rx, ry2 = ry * ry;
        int tworx2 = 2 * rx2, twory2 = 2 * ry2;
        int p = ry2 - rx2 * ry + (rx2 >> 2);
        int px = 0, py = tworx2 * y;

        while (px < py) {
            setPixel(cx + x, cy + y, color);
            setPixel(cx - x, cy + y, color);
            setPixel(cx + x, cy - y, color);
            setPixel(cx - x, cy - y, color);
            x++;
            px += twory2;
            if (p < 0)
                p += ry2 + px;
            else {
                y--;
                py -= tworx2;
                p += ry2 + px - py;
            }
        }

        p = ry2 * (x + 0.5) * (x + 0.5) + rx2 * (y - 1) * (y - 1) - rx2 * ry2;
        while (y >= 0) {
            setPixel(cx + x, cy + y, color);
            setPixel(cx - x, cy + y, color);
            setPixel(cx + x, cy - y, color);
            setPixel(cx - x, cy - y, color);
            y--;
            py -= tworx2;
            if (p > 0)
                p += rx2 - py;
            else {
                x++;
                px += twory2;
                p += rx2 - py + px;
            }
        }
    } else if (m_currentTool == Line) {
        int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy, e2;
        for (;;) {
            setPixel(x0, y0, color);
            if (x0 == x1 && y0 == y1) break;
            e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    } else if (m_currentTool == Triangle) {
        int x2 = x0, y2 = y1;
        QPoint p0(x0, y0), p1(x1, y1), p2(x2, y2);
        auto drawLine = [&](QPoint a, QPoint b) {
            int dx = abs(b.x() - a.x()), sx = a.x() < b.x() ? 1 : -1;
            int dy = -abs(b.y() - a.y()), sy = a.y() < b.y() ? 1 : -1;
            int err = dx + dy, e2;
            int x = a.x(), y = a.y();
            for (;;) {
                setPixel(x, y, color);
                if (x == b.x() && y == b.y()) break;
                e2 = 2 * err;
                if (e2 >= dy) { err += dy; x += sx; }
                if (e2 <= dx) { err += dx; y += sy; }
            }
        };
        drawLine(p0, p1);
        drawLine(p1, p2);
        drawLine(p2, p0);
    }

    update();
    emit canvasChanged();
}

void CanvasWidget::drawShapePreview(QPainter &painter)
{
    if (!m_shapeActive) return;

    QPoint start = m_shapeStart;
    QPoint end = m_shapeCurrent;
    QColor color(m_currentRgb.red(), m_currentRgb.green(), m_currentRgb.blue(), 128);

    auto drawPixel = [&](int x, int y) {
        if (x >= 0 && x < m_canvasSize.width() && y >= 0 && y < m_canvasSize.height()) {
            painter.fillRect(x * m_pixelSize, y * m_pixelSize, m_pixelSize, m_pixelSize, color);
        }
    };

    int x0 = start.x(), y0 = start.y();
    int x1 = end.x(), y1 = end.y();

    if (m_currentTool == Rectangle) {
        int xmin = qMin(x0, x1), xmax = qMax(x0, x1);
        int ymin = qMin(y0, y1), ymax = qMax(y0, y1);
        for (int x = xmin; x <= xmax; ++x) {
            drawPixel(x, ymin);
            drawPixel(x, ymax);
        }
        for (int y = ymin; y <= ymax; ++y) {
            drawPixel(xmin, y);
            drawPixel(xmax, y);
        }
    } else if (m_currentTool == Ellipse) {
        int cx = (x0 + x1) / 2;
        int cy = (y0 + y1) / 2;
        int rx = qAbs(x1 - x0) / 2;
        int ry = qAbs(y1 - y0) / 2;
        if (rx == 0 || ry == 0) return;

        int x = 0, y = ry;
        int rx2 = rx * rx, ry2 = ry * ry;
        int tworx2 = 2 * rx2, twory2 = 2 * ry2;
        int p = ry2 - rx2 * ry + (rx2 >> 2);
        int px = 0, py = tworx2 * y;

        while (px < py) {
            drawPixel(cx + x, cy + y);
            drawPixel(cx - x, cy + y);
            drawPixel(cx + x, cy - y);
            drawPixel(cx - x, cy - y);
            x++;
            px += twory2;
            if (p < 0)
                p += ry2 + px;
            else {
                y--;
                py -= tworx2;
                p += ry2 + px - py;
            }
        }

        p = ry2 * (x + 0.5) * (x + 0.5) + rx2 * (y - 1) * (y - 1) - rx2 * ry2;
        while (y >= 0) {
            drawPixel(cx + x, cy + y);
            drawPixel(cx - x, cy + y);
            drawPixel(cx + x, cy - y);
            drawPixel(cx - x, cy - y);
            y--;
            py -= tworx2;
            if (p > 0)
                p += rx2 - py;
            else {
                x++;
                px += twory2;
                p += rx2 - py + px;
            }
        }
    } else if (m_currentTool == Line) {
        int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy, e2;
        for (;;) {
            drawPixel(x0, y0);
            if (x0 == x1 && y0 == y1) break;
            e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    } else if (m_currentTool == Triangle) {
        int x2 = x0, y2 = y1;
        QPoint p0(x0, y0), p1(x1, y1), p2(x2, y2);
        auto drawLine = [&](QPoint a, QPoint b) {
            int dx = abs(b.x() - a.x()), sx = a.x() < b.x() ? 1 : -1;
            int dy = -abs(b.y() - a.y()), sy = a.y() < b.y() ? 1 : -1;
            int err = dx + dy, e2;
            int x = a.x(), y = a.y();
            for (;;) {
                drawPixel(x, y);
                if (x == b.x() && y == b.y()) break;
                e2 = 2 * err;
                if (e2 >= dy) { err += dy; x += sx; }
                if (e2 <= dx) { err += dx; y += sy; }
            }
        };
        drawLine(p0, p1);
        drawLine(p1, p2);
        drawLine(p2, p0);
    }
}

void CanvasWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    drawCheckerboard(painter);

    if (!m_ghostImage.isNull()) {
        for (int y = 0; y < m_canvasSize.height(); ++y) {
            for (int x = 0; x < m_canvasSize.width(); ++x) {
                QColor col = m_ghostImage.pixelColor(x, y);
                if (col.alpha() > 0) {
                    painter.fillRect(x * m_pixelSize, y * m_pixelSize,
                                     m_pixelSize, m_pixelSize, col);
                }
            }
        }
    }

    for (int y = 0; y < m_canvasSize.height(); ++y) {
        for (int x = 0; x < m_canvasSize.width(); ++x) {
            QColor col = m_image.pixelColor(x, y);
            if (col.alpha() > 0) {
                painter.fillRect(x * m_pixelSize, y * m_pixelSize,
                                 m_pixelSize, m_pixelSize, col);
            }
        }
    }

    drawShapePreview(painter);

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

        if (m_currentTool == Rectangle || m_currentTool == Ellipse ||
            m_currentTool == Line || m_currentTool == Triangle) {
            m_shapeStart = pixel;
            m_shapeCurrent = pixel;
            m_shapeActive = true;
            update();
        } else {
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
                m_drawing = false;
                break;
            case Picker:
                pickColorAt(pixel);
                m_drawing = false;
                break;
            default:
                break;
            }
        }
    }
}

void CanvasWidget::mouseMoveEvent(QMouseEvent *event)
{
    QPoint pixel = pixelFromPoint(event->pos());
    if (pixel.x() < 0) return;

    if (m_shapeActive) {
        m_shapeCurrent = pixel;
        update();
        return;
    }

    if (!m_drawing || !(event->buttons() & Qt::LeftButton))
        return;

    if (m_currentTool == Brush || m_currentTool == Eraser) {
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
        if (m_shapeActive) {
            applyShape(m_shapeStart, m_shapeCurrent);
            m_shapeActive = false;
            update();
        }
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

void CanvasWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    emit sizeChanged(size());
}