#ifndef COLORPALETTE_H
#define COLORPALETTE_H

#include <QWidget>
#include <QColor>

class CanvasWidget; // предварительное объявление

class ColorPalette: public QWidget {
    Q_OBJECT

    public:
        explicit ColorPalette(CanvasWidget *canvas, QWidget *parent = nullptr);

    private slots:
        void onColorSelected(const QColor &color);

    private:
        CanvasWidget *m_canvas;
};

#endif // COLORPALETTE_H