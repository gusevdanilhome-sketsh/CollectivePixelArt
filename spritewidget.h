#ifndef SPRITEWIDGET_H
#define SPRITEWIDGET_H

#include <QWidget>
#include <QImage>
#include <QVector>
#include <QColor>

// Виджет для отображения одного кадра анимации (без возможности редактирования)
class SpriteWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SpriteWidget(QWidget *parent = nullptr);

    // Установить кадр из двумерного массива цветов
    void setFrame(const QVector<QVector<QColor>> &pixels);

    // Очистить виджет (показать серый фон и надпись "Нет кадра")
    void clear();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QImage m_image;          // Изображение кадра в исходном разрешении
    QSize m_frameSize;       // Размер кадра (ширина/высота в пикселях)
    int m_pixelSize = 10;    // Масштаб при отрисовке (можно сделать настраиваемым)
};

#endif // SPRITEWIDGET_H