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
    setFixedSize(400, 400); // временный размер, потом можно настроить
    setCanvasSize(32, 32);  // холст 32x32 пикселя
}

void CanvasWidget::setPixelSize(int size) {
    m_pixelSize = size;
    setFixedSize(m_canvasSize.width() * m_pixelSize,
                 m_canvasSize.height() * m_pixelSize);
    update();
}

void CanvasWidget::setCanvasSize(int width, int height) {
    m_canvasSize = QSize(width, height);
    m_pixels.resize(height);
    for (int i = 0; i < height; ++i) {
        m_pixels[i].resize(width);
        for (int j = 0; j < width; ++j) {
            m_pixels[i][j] = Qt::white; // фон белый
        }
    }
    setFixedSize(width * m_pixelSize, height * m_pixelSize);
    update();
}

void CanvasWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);

    // Рисуем пиксели
    for (int y = 0; y < m_canvasSize.height(); ++y) {
        for (int x = 0; x < m_canvasSize.width(); ++x) {
            painter.fillRect(x * m_pixelSize, y * m_pixelSize,
                             m_pixelSize, m_pixelSize, m_pixels[y][x]);
        }
    }

    // Рисуем сетку (серые линии)
    painter.setPen(QPen(Qt::lightGray, 1));
    for (int x = 0; x <= m_canvasSize.width(); ++x) {
        painter.drawLine(x * m_pixelSize, 0,
                         x * m_pixelSize, height());
    }
    for (int y = 0; y <= m_canvasSize.height(); ++y) {
        painter.drawLine(0, y * m_pixelSize,
                         width(), y * m_pixelSize);
    }
}

QPoint CanvasWidget::pixelFromPoint(const QPoint &point) const {
    int x = point.x() / m_pixelSize;
    int y = point.y() / m_pixelSize;
    if (x < 0 || x >= m_canvasSize.width() ||
        y < 0 || y >= m_canvasSize.height()) {
        return QPoint(-1, -1);
    }
    return QPoint(x, y);
}

void CanvasWidget::drawPixel(const QPoint &pixelPos) {
    if (pixelPos.x() < 0 || pixelPos.y() < 0)
        return;
    m_pixels[pixelPos.y()][pixelPos.x()] = m_currentColor;
    update();
}

void CanvasWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_drawing = true;
        drawPixel(pixelFromPoint(event->pos()));
    }
}

void CanvasWidget::mouseMoveEvent(QMouseEvent *event) {
    if (m_drawing && (event->buttons() & Qt::LeftButton)) {
        drawPixel(pixelFromPoint(event->pos()));
    }
}

void CanvasWidget::wheelEvent(QWheelEvent *event) {
    if (event->modifiers() & Qt::ControlModifier) {
        int delta = event->angleDelta().y();
        int newSize = m_pixelSize + (delta > 0 ? 1 : -1);
        newSize = qBound(4, newSize, 64); // ограничиваем от 4 до 64

        if (newSize != m_pixelSize) {
            setPixelSize(newSize);

            // Обновляем родительский QScrollArea, чтобы полосы прокрутки пересчитались
            if (parentWidget()) {
                QScrollArea *scrollArea = qobject_cast<QScrollArea*>(parentWidget());
                if (scrollArea) {
                    scrollArea->updateGeometry();
                }
            }
        }
        event->accept();
    } else {
        // Передаём событие дальше для обычной прокрутки
        event->ignore();
    }
}