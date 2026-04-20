#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Никакой дополнительной инициализации — только пустое окно
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ---------- Кадры ----------
void MainWindow::on_AddFramesButton_clicked() {}
void MainWindow::on_ClearFramesButton_clicked() {}
void MainWindow::on_PrevFrameButton_clicked() {}
void MainWindow::on_NextFrameButton_clicked() {}
void MainWindow::on_DeletFrameButton_clicked() {}

// ---------- Анимация ----------
void MainWindow::on_StattButton_clicked() {}
void MainWindow::on_StopButton_clicked() {}
void MainWindow::on_num_fps_valueChanged(int /*fps*/) {}

// ---------- Файлы ----------
void MainWindow::on_ChangingDirectoryButton_clicked() {}
void MainWindow::on_UpdateButton_clicked() {}
void MainWindow::on_SearchFile_textChanged(const QString & /*text*/) {}

// ---------- Слои ----------
void MainWindow::on_IncrementButton_clicked() {}
void MainWindow::on_DecrementButton_clicked() {}
void MainWindow::on_VisibilityCheck_toggled(bool /*checked*/) {}
void MainWindow::on_TransparencySlider_valueChanged(int /*value*/) {}

// ---------- Инструменты ----------
void MainWindow::on_BrushButton_clicked() {}
void MainWindow::on_EraserButton_clicked() {}
void MainWindow::on_FillButton_clicked() {}
void MainWindow::on_PickerButton_clicked() {}
void MainWindow::on_BrushSizeSpin_valueChanged(int /*size*/) {}

// ---------- Цвет ----------
void MainWindow::on_BitDepthSpin_valueChanged(int /*size*/) {}
void MainWindow::on_AlfaChannelSpin_valueChanged(int /*value*/) {}

// ---------- Экспорт/импорт ----------
void MainWindow::on_ExportButton_clicked() {}
void MainWindow::on_ImportButton_clicked() {}