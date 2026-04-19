#ifndef COLORPALETTE_H
#define COLORPALETTE_H

#include <QWidget>
#include <QColor>
#include <QGridLayout>

class CanvasWidget;

class ColorPalette : public QWidget
{
    Q_OBJECT
public:
    explicit ColorPalette(CanvasWidget *canvas, QWidget *parent = nullptr);
    void setPaletteSize(int size);   // размер сетки size x size

private slots:
    void onColorSelected(const QColor &color);

private:
    void generatePalette();
    QColor getColorForIndex(int row, int col, int size) const;

    CanvasWidget *m_canvas;
    int m_paletteSize = 4;
    QGridLayout *m_layout;
};

#endif // COLORPALETTE_H