#ifndef NAVIGATIONWIDGET_H
#define NAVIGATIONWIDGET_H

#include <QWidget>

class CanvasWidget;

class NavigationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NavigationWidget(QWidget *parent = nullptr);

    void setCanvasWidget(CanvasWidget *canvas);

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void updateViewport();

private:
    CanvasWidget *m_canvas = nullptr;
    qreal m_scale = 1.0;
    QRectF m_viewportRect;
    bool m_dragging = false;
    QPointF m_dragStartPos;

    void computeViewportRect();
    QPointF mapToCanvas(const QPointF &pos) const;
};

#endif // NAVIGATIONWIDGET_H