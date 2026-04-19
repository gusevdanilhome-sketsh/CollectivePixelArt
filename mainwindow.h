#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QLabel>
#include <QTimer>
#include "canvaswidget.h"
#include "colorpalette.h"
#include "spritewidget.h"   // <-- новый заголовок

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
    void onAddFrame();
    void onClearFrame();
    void onPreviousFrame();
    void onNextFrame();

    void onPlayAnimation();
    void onStopAnimation();
    void onAnimationTick();
    void onFpsChanged(int fps);

    void updateStatusBar(const QColor &color);

private:
    void setupFrameLine();
    void updateFrameLine();
    void loadFrame(int index);
    void setupSpriteView();

    Ui::MainWindow *ui;
    CanvasWidget *m_canvasWidget;
    ColorPalette *m_colorPalette;
    SpriteWidget *m_spriteWidget;   // <-- виджет для предпросмотра

    QList<QVector<QVector<QColor>>> m_frames;
    int m_currentFrameIndex = -1;

    QLabel *m_statusColorLabel;
    QLabel *m_statusColorIcon;

    QTimer *m_animationTimer;
    int m_animationFps = 12;
};

#endif // MAINWINDOW_H