#include "navigationwidget.h"
#include "canvaswidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QScrollArea>
#include <QScrollBar>

NavigationWidget::NavigationWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(100, 100);
}

void NavigationWidget::setCanvasWidget(CanvasWidget *canvas)
{
    if (m_canvas) {
        disconnect(m_canvas, nullptr, this, nullptr);
    }

    m_canvas = canvas;

    if (m_canvas) {
        // Подключаемся к изменениям холста и области прокрутки
        connect(m_canvas, &CanvasWidget::canvasChanged, this, &NavigationWidget::updateViewport);
        connect(m_canvas, &CanvasWidget::viewportChanged, this, &NavigationWidget::updateViewport);
        // Также нужно следить за изменениями размера пикселя (масштаб)
        connect(m_canvas, &CanvasWidget::pixelSizeChanged, this, &NavigationWidget::updateViewport);
        // При изменении размера виджета холста
        connect(m_canvas, &CanvasWidget::sizeChanged, this, &NavigationWidget::updateViewport);

        // Если холст внутри QScrollArea, отслеживаем скролл
        if (QScrollArea *scrollArea = qobject_cast<QScrollArea*>(m_canvas->parentWidget())) {
            connect(scrollArea->horizontalScrollBar(), &QScrollBar::valueChanged,
                    this, &NavigationWidget::updateViewport);
            connect(scrollArea->verticalScrollBar(), &QScrollBar::valueChanged,
                    this, &NavigationWidget::updateViewport);
        }
    }
    updateViewport();
}

QSize NavigationWidget::minimumSizeHint() const
{
    return QSize(100, 100);
}

QSize NavigationWidget::sizeHint() const
{
    return QSize(150, 150);
}

void NavigationWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    painter.fillRect(rect(), QColor(60, 60, 60));

    if (!m_canvas) {
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, tr("Нет холста"));
        return;
    }

    QSize canvasSize = m_canvas->canvasSize();
    if (canvasSize.isEmpty()) return;

    qreal scaleX = static_cast<qreal>(width()) / canvasSize.width();
    qreal scaleY = static_cast<qreal>(height()) / canvasSize.height();
    m_scale = qMin(scaleX, scaleY);

    QSize thumbnailSize(canvasSize.width() * m_scale, canvasSize.height() * m_scale);
    QPoint offset((width() - thumbnailSize.width()) / 2,
                  (height() - thumbnailSize.height()) / 2);

    // Получаем изображение через image()
    QImage frameImage = m_canvas->image();
    if (!frameImage.isNull()) {
        QImage scaled = frameImage.scaled(thumbnailSize, Qt::KeepAspectRatio, Qt::FastTransformation);
        painter.drawImage(offset, scaled);
    } else {
        // Рисуем шахматную доску, если изображения нет
        const int cellSize = 5;
        for (int y = 0; y < thumbnailSize.height(); y += cellSize) {
            for (int x = 0; x < thumbnailSize.width(); x += cellSize) {
                bool white = ((x / cellSize) + (y / cellSize)) % 2 == 0;
                painter.fillRect(offset.x() + x, offset.y() + y, cellSize, cellSize,
                                 white ? Qt::lightGray : Qt::gray);
            }
        }
    }

    painter.setPen(QPen(Qt::gray, 1));
    painter.drawRect(offset.x(), offset.y(), thumbnailSize.width(), thumbnailSize.height());

    computeViewportRect();
    m_viewportRect.translate(offset);
    painter.setPen(QPen(Qt::red, 2));
    painter.drawRect(m_viewportRect);
    m_viewportRect.translate(-offset);
}

void NavigationWidget::computeViewportRect()
{
    if (!m_canvas) {
        m_viewportRect = QRectF();
        return;
    }

    QSize canvasSize = m_canvas->canvasSize();
    if (canvasSize.isEmpty()) {
        m_viewportRect = QRectF();
        return;
    }

    // Получаем видимую область в пикселях холста
    QScrollArea *scrollArea = qobject_cast<QScrollArea*>(m_canvas->parentWidget());
    QRect visibleRect;
    if (scrollArea) {
        QPoint scrollPos(scrollArea->horizontalScrollBar()->value(),
                         scrollArea->verticalScrollBar()->value());
        QSize viewportSize = scrollArea->viewport()->size();
        visibleRect = QRect(scrollPos, viewportSize);
    } else {
        // Если нет скролла, видимая область - весь холст
        visibleRect = QRect(QPoint(0,0), m_canvas->size());
    }

    // Преобразуем видимую область в координаты пикселей холста (индексы)
    int pixelSize = m_canvas->pixelSize();
    qreal left = static_cast<qreal>(visibleRect.left()) / pixelSize;
    qreal top = static_cast<qreal>(visibleRect.top()) / pixelSize;
    qreal width = static_cast<qreal>(visibleRect.width()) / pixelSize;
    qreal height = static_cast<qreal>(visibleRect.height()) / pixelSize;

    // Ограничиваем размерами холста
    left = qMax(0.0, left);
    top = qMax(0.0, top);
    width = qMin(width, static_cast<qreal>(canvasSize.width()) - left);
    height = qMin(height, static_cast<qreal>(canvasSize.height()) - top);

    // Применяем масштаб миниатюры
    m_viewportRect = QRectF(left * m_scale, top * m_scale, width * m_scale, height * m_scale);
}

void NavigationWidget::updateViewport()
{
    update();
}

QPointF NavigationWidget::mapToCanvas(const QPointF &pos) const
{
    if (!m_canvas) return QPointF();

    QSize canvasSize = m_canvas->canvasSize();
    if (canvasSize.isEmpty()) return QPointF();

    // Вычисляем смещение миниатюры
    qreal scaleX = static_cast<qreal>(width()) / canvasSize.width();
    qreal scaleY = static_cast<qreal>(height()) / canvasSize.height();
    qreal scale = qMin(scaleX, scaleY);
    QSize thumbnailSize(canvasSize.width() * scale, canvasSize.height() * scale);
    QPoint offset((width() - thumbnailSize.width()) / 2,
                  (height() - thumbnailSize.height()) / 2);

    // Переводим в координаты миниатюры
    QPointF thumbnailPos = pos - offset;
    // Переводим в координаты холста (индексы пикселей)
    qreal canvasX = thumbnailPos.x() / scale;
    qreal canvasY = thumbnailPos.y() / scale;

    // Ограничиваем
    canvasX = qBound(0.0, canvasX, static_cast<qreal>(canvasSize.width()));
    canvasY = qBound(0.0, canvasY, static_cast<qreal>(canvasSize.height()));

    return QPointF(canvasX, canvasY);
}

void NavigationWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_canvas) {
        QPointF canvasPos = mapToCanvas(event->pos());
        // Проверяем, попали ли в видимую область
        if (m_viewportRect.contains(event->pos())) {
            m_dragging = true;
            m_dragStartPos = canvasPos;
        } else {
            // Перемещаем центр видимой области в точку клика
            QScrollArea *scrollArea = qobject_cast<QScrollArea*>(m_canvas->parentWidget());
            if (scrollArea) {
                int pixelSize = m_canvas->pixelSize();
                QPoint targetPos(canvasPos.x() * pixelSize - scrollArea->viewport()->width() / 2,
                                 canvasPos.y() * pixelSize - scrollArea->viewport()->height() / 2);
                scrollArea->horizontalScrollBar()->setValue(targetPos.x());
                scrollArea->verticalScrollBar()->setValue(targetPos.y());
            }
        }
        event->accept();
    } else {
        QWidget::mousePressEvent(event);
    }
}

void NavigationWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && m_canvas) {
        QPointF canvasPos = mapToCanvas(event->pos());
        QScrollArea *scrollArea = qobject_cast<QScrollArea*>(m_canvas->parentWidget());
        if (scrollArea) {
            int pixelSize = m_canvas->pixelSize();
            QPoint delta((canvasPos.x() - m_dragStartPos.x()) * pixelSize,
                         (canvasPos.y() - m_dragStartPos.y()) * pixelSize);
            scrollArea->horizontalScrollBar()->setValue(scrollArea->horizontalScrollBar()->value() + delta.x());
            scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->value() + delta.y());
            m_dragStartPos = canvasPos;
        }
        event->accept();
    } else {
        QWidget::mouseMoveEvent(event);
    }
}

void NavigationWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_dragging) {
        m_dragging = false;
        event->accept();
    } else {
        QWidget::mouseReleaseEvent(event);
    }
}