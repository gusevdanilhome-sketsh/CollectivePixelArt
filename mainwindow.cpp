#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "canvaswidget.h"
#include "colorpalette.h"
#include <QHBoxLayout>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // Получаем контейнер внутри QScrollArea (он называется canvasContainer)
    QWidget *container = ui->scrollAreaCanvas->widget();
    if (!container) {
        container = new QWidget(ui->scrollAreaCanvas);
        ui->scrollAreaCanvas->setWidget(container);
    }

    // Создаём холст
    m_canvasWidget = new CanvasWidget(container);
    m_canvasWidget->setCanvasSize(32, 32);
    m_canvasWidget->setPixelSize(16);

    // Размещаем холст по центру контейнера
    QHBoxLayout *layout = new QHBoxLayout(container);
    layout->addWidget(m_canvasWidget);
    layout->setAlignment(m_canvasWidget, Qt::AlignCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    container->setLayout(layout);

    // Создаём палитру цветов и размещаем на вкладке "colors"
    m_colorPalette = new ColorPalette(m_canvasWidget, ui->colors);
    QVBoxLayout *paletteLayout = new QVBoxLayout(ui->colors);
    paletteLayout->addWidget(m_colorPalette);
    ui->colors->setLayout(paletteLayout);
}

MainWindow::~MainWindow()
{
    delete ui;
}