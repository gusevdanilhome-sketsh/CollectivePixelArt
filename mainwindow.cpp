#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "canvaswidget.h"
#include "colorpalette.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpinBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // --- Холст и QScrollArea ---
    // В .ui scrollAreaCanvas содержит виджет m_canvas
    QWidget *container = ui->m_canvas;
    if (!container) {
        container = new QWidget(ui->scrollAreaCanvas);
        ui->scrollAreaCanvas->setWidget(container);
    }

    m_canvasWidget = new CanvasWidget(container);
    m_canvasWidget->setCanvasSize(32, 32);
    m_canvasWidget->setPixelSize(16);

    QHBoxLayout *canvasLayout = new QHBoxLayout(container);
    canvasLayout->addWidget(m_canvasWidget);
    canvasLayout->setAlignment(m_canvasWidget, Qt::AlignCenter);
    canvasLayout->setContentsMargins(0, 0, 0, 0);
    container->setLayout(canvasLayout);

    // --- Палитра цветов ---
    QWidget *paletteContainer = ui->palette;   // виджет-контейнер из .ui
    if (!paletteContainer) {
        paletteContainer = new QWidget(ui->colors);
        ui->gridLayout_3->addWidget(paletteContainer, 2, 0);
    }

    m_colorPalette = new ColorPalette(m_canvasWidget, paletteContainer);
    QVBoxLayout *paletteLayout = new QVBoxLayout(paletteContainer);
    paletteLayout->addWidget(m_colorPalette);
    paletteLayout->setContentsMargins(0, 0, 0, 0);
    paletteContainer->setLayout(paletteLayout);

    // Связь spinBox "palettesize" с размером палитры
    QSpinBox *sizeSpinBox = ui->palettesize;
    if (sizeSpinBox) {
        connect(sizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                m_colorPalette, &ColorPalette::setPaletteSize);
        m_colorPalette->setPaletteSize(sizeSpinBox->value());
    } else {
        m_colorPalette->setPaletteSize(4);
    }

    // --- Альфа-канал (spinBox на вкладке colors) ---
    QSpinBox *alphaSpinBox = ui->spinBox;
    if (alphaSpinBox) {
        alphaSpinBox->setRange(0, 255);
        alphaSpinBox->setValue(255);
        connect(alphaSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                m_canvasWidget, &CanvasWidget::setAlpha);
        m_canvasWidget->setAlpha(alphaSpinBox->value());
    } else {
        m_canvasWidget->setAlpha(255);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}