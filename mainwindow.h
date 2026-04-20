#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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
    // Кадры
    void on_AddFramesButton_clicked();
    void on_ClearFramesButton_clicked();
    void on_PrevFrameButton_clicked();
    void on_NextFrameButton_clicked();
    void on_DeletFrameButton_clicked();

    // Анимация
    void on_StattButton_clicked();
    void on_StopButton_clicked();
    void on_num_fps_valueChanged(int fps);

    // Файлы
    void on_ChangingDirectoryButton_clicked();
    void on_UpdateButton_clicked();
    void on_SearchFile_textChanged(const QString &text);

    // Слои
    void on_IncrementButton_clicked();
    void on_DecrementButton_clicked();
    void on_VisibilityCheck_toggled(bool checked);
    void on_TransparencySlider_valueChanged(int value);

    // Инструменты (если нужны действия, иначе можно не подключать)
    void on_BrushButton_clicked();
    void on_EraserButton_clicked();
    void on_FillButton_clicked();
    void on_PickerButton_clicked();
    void on_BrushSizeSpin_valueChanged(int size);

    // Цвет
    void on_BitDepthSpin_valueChanged(int size);
    void on_AlfaChannelSpin_valueChanged(int value);

    // Экспорт/импорт
    void on_ExportButton_clicked();
    void on_ImportButton_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H