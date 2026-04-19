#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QLabel>
#include <QTimer>
#include <QButtonGroup>
#include <QListWidget>
#include <QSlider>
#include "canvaswidget.h"
#include "colorpalette.h"
#include "spritewidget.h"
#include "frame.h"
#include "navigationwidget.h"

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

    void onToolButtonClicked(int id);

    void updateStatusBar(const QColor &color);
    void updateToolStatus(CanvasWidget::Tool tool);

    void onNewLayer();
    void onDeleteLayer();
    void onLayerSelectionChanged();
    void onLayerVisibilityChanged(bool visible);
    void onLayerOpacityChanged(int opacity);
    void updateLayerList();

    void onNewProject();
    void onSaveProject();
    void onLoadProject();
    void onExportSpriteSheet();

    void onCanvasChanged();

private:
    void setupFrameLine();
    void updateFrameLine();
    void loadFrame(int index);
    void setupSpriteView();
    void setupToolButtons();
    void createMenuBar();
    void createStatusBar();
    void setupLayerPanel();      // теперь настраивает вкладку LayerTabe
    void setCurrentFrame(Frame *frame);
    Frame* currentFrame() const;
    Layer* currentLayer() const;
    void refreshCanvasFromFrame();

    Ui::MainWindow *ui;
    CanvasWidget *m_canvasWidget;
    ColorPalette *m_colorPalette;
    SpriteWidget *m_spriteWidget;
    NavigationWidget *m_navigationWidget;

    QList<Frame*> m_frames;
    int m_currentFrameIndex = -1;
    QSize m_canvasSize = QSize(32, 32);

    // Элементы панели слоёв (используем прямо из UI)
    QListWidget *m_layerList;          // создаётся динамически и помещается в ui->frame

    QLabel *m_statusColorLabel;
    QLabel *m_statusColorIcon;
    QLabel *m_statusToolLabel;

    QTimer *m_animationTimer;
    int m_animationFps = 12;

    QButtonGroup *m_toolButtonGroup;
};

#endif // MAINWINDOW_H