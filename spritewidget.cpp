#include "spritewidget.h"
#include <QPainter>
#include <QWheelEvent>

SpriteWidget::SpriteWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(100, 100);
    m_pixelSize = 10;
}

void SpriteWidget::setFrame(const QImage &image)
{
    if (image.isNull()) {
        clear();
        return;
    }

    m_image = image;
    update();
}

void SpriteWidget::clear()
{
    m_image = QImage();
    update();
}

void SpriteWidget::setPixelSize(int size)
{
    m_pixelSize = qBound(4, size, 64);
    update();
}

void SpriteWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);

    if (m_image.isNull()) {
        painter.fillRect(rect(), Qt::lightGray);
        painter.setPen(Qt::darkGray);
        painter.setFont(QFont("Arial", 12));
        painter.drawText(rect(), Qt::AlignCenter, tr("Нет кадра"));
        return;
    }

    QSize imageSize = m_image.size();
    QSize scaledSize = imageSize * m_pixelSize;

    QRect targetRect(0, 0, scaledSize.width(), scaledSize.height());
    targetRect.moveCenter(rect().center());

    drawCheckerboard(painter, targetRect);
    painter.drawImage(targetRect, m_image);

    painter.setPen(QPen(Qt::gray, 1));
    painter.drawRect(targetRect);
}

void SpriteWidget::drawCheckerboard(QPainter &painter, const QRect &rect)
{
    const int cellSize = m_pixelSize;
    int cellsX = rect.width() / cellSize + 1;
    int cellsY = rect.height() / cellSize + 1;

    for (int y = 0; y < cellsY; ++y) {
        for (int x = 0; x < cellsX; ++x) {
            bool isLight = ((x + y) % 2 == 0);
            int cellX = rect.x() + x * cellSize;
            int cellY = rect.y() + y * cellSize;
            int cellW = qMin(cellSize, rect.x() + rect.width() - cellX);
            int cellH = qMin(cellSize, rect.y() + rect.height() - cellY);

            if (cellW > 0 && cellH > 0) {
                painter.fillRect(cellX, cellY, cellW, cellH,
                                 isLight ? Qt::lightGray : Qt::gray);
            }
        }
    }
}

void SpriteWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
}

void SpriteWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    QWidget::mouseDoubleClickEvent(event);
}

void SpriteWidget::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        int delta = event->angleDelta().y();
        int newSize = m_pixelSize + (delta > 0 ? 1 : -1);
        setPixelSize(newSize);
        event->accept();
    } else {
        QWidget::wheelEvent(event);
    }
}

QSize SpriteWidget::sizeHint() const
{
    if (m_image.isNull()) {
        return QSize(200, 200);
    }
    QSize imageSize = m_image.size();
    return QSize(imageSize.width() * m_pixelSize + 20,
                 imageSize.height() * m_pixelSize + 20);
}

QSize SpriteWidget::minimumSizeHint() const
{
    return QSize(100, 100);
}