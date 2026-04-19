#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QLabel>
#include "canvaswidget.h"
#include "colorpalette.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    // Слоты для кнопок управления кадрами
    void onAddFrame();        // Добавить текущий кадр
    void onClearFrame();      // Очистить холст
    void onPreviousFrame();   // Перейти к предыдущему кадру
    void onNextFrame();       // Перейти к следующему кадру

    // Обновление статус-бара при смене цвета
    void updateStatusBar(const QColor &color);

private:
    void setupFrameLine();    // Инициализация отображения кадров
    void updateFrameLine();   // Обновить миниатюры кадров в frameline
    void loadFrame(int index);// Загрузить кадр по индексу в холст

    Ui::MainWindow *ui;
    CanvasWidget *m_canvasWidget;
    ColorPalette *m_colorPalette;

    // Хранилище кадров (копии массивов пикселей)
    QList<QVector<QVector<QColor>>> m_frames;
    int m_currentFrameIndex = -1; // Индекс текущего отображаемого кадра, -1 если не выбран

    // Элементы статус-бара
    QLabel *m_statusColorLabel;   // Текст с кодом цвета
    QLabel *m_statusColorIcon;    // Иконка цвета
};

#endif // MAINWINDOW_H