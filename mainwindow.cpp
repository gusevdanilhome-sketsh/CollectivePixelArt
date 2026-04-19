#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "canvaswidget.h"
#include "colorpalette.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QPushButton>
#include <QListWidget>
#include <QStatusBar>
#include <QPainter>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // --- Настройка холста ---
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
    QWidget *paletteContainer = ui->palette;
    if (!paletteContainer) {
        paletteContainer = new QWidget(ui->colors);
        ui->gridLayout_3->addWidget(paletteContainer, 2, 0);
    }

    m_colorPalette = new ColorPalette(m_canvasWidget, paletteContainer);
    QVBoxLayout *paletteLayout = new QVBoxLayout(paletteContainer);
    paletteLayout->addWidget(m_colorPalette);
    paletteLayout->setContentsMargins(0, 0, 0, 0);
    paletteContainer->setLayout(paletteLayout);

    // Размер палитры (максимум 32)
    QSpinBox *sizeSpinBox = ui->palettesize;
    if (sizeSpinBox) {
        sizeSpinBox->setRange(1, 32); // Было 16, стало 32
        connect(sizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                m_colorPalette, &ColorPalette::setPaletteSize);
        m_colorPalette->setPaletteSize(sizeSpinBox->value());
    } else {
        m_colorPalette->setPaletteSize(4);
    }

    // Альфа-канал
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

    // --- Настройка кнопок управления кадрами ---
    // В UI добавлены кнопки pushButton, pushButton_2, pushButton_3, pushButton_4
    // Дадим им осмысленные имена и подключим слоты
    QPushButton *btnAddFrame = ui->addframes;      // Добавить кадр
    QPushButton *btnClearFrame = ui->clearframes;  // Очистить
    QPushButton *btnPrevFrame = ui->prevframe;   // Предыдущий
    QPushButton *btnNextFrame = ui->nextframe;   // Следующий

    connect(btnAddFrame, &QPushButton::clicked, this, &MainWindow::onAddFrame);
    connect(btnClearFrame, &QPushButton::clicked, this, &MainWindow::onClearFrame);
    connect(btnPrevFrame, &QPushButton::clicked, this, &MainWindow::onPreviousFrame);
    connect(btnNextFrame, &QPushButton::clicked, this, &MainWindow::onNextFrame);

    // --- Настройка статус-бара ---
    m_statusColorLabel = new QLabel(this);
    m_statusColorIcon = new QLabel(this);
    m_statusColorIcon->setFixedSize(20, 20);
    statusBar()->addPermanentWidget(m_statusColorIcon);
    statusBar()->addPermanentWidget(m_statusColorLabel);
    // Подключаем сигнал изменения цвета от холста
    connect(m_canvasWidget, &CanvasWidget::currentColorChanged, this, &MainWindow::updateStatusBar);
    // Инициализируем отображение текущего цвета
    updateStatusBar(QColor(m_canvasWidget->currentColor().red(),
                           m_canvasWidget->currentColor().green(),
                           m_canvasWidget->currentColor().blue(),
                           m_canvasWidget->alpha()));

    // --- Инициализация отображения кадров (frameline) ---
    setupFrameLine();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupFrameLine()
{
    // Создадим QListWidget для отображения миниатюр кадров внутри вкладки frameline
    QWidget *framelineTab = ui->frameline;
    QVBoxLayout *layout = new QVBoxLayout(framelineTab);
    QListWidget *frameList = new QListWidget(framelineTab);
    frameList->setViewMode(QListWidget::IconMode);
    frameList->setIconSize(QSize(64, 64));
    frameList->setResizeMode(QListWidget::Adjust);
    frameList->setMovement(QListWidget::Static);
    layout->addWidget(frameList);
    framelineTab->setLayout(layout);

    // Сохраним указатель на список как член класса? Для простоты будем искать по имени
    frameList->setObjectName("frameListWidget");
}

void MainWindow::updateFrameLine()
{
    QListWidget *frameList = ui->frameline->findChild<QListWidget*>("frameListWidget");
    if (!frameList) return;

    frameList->clear();
    for (int i = 0; i < m_frames.size(); ++i) {
        // Создаём QImage из массива пикселей
        const auto &pixels = m_frames[i];
        int h = pixels.size();
        int w = h > 0 ? pixels[0].size() : 0;
        QImage image(w, h, QImage::Format_ARGB32);
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                image.setPixelColor(x, y, pixels[y][x]);
            }
        }
        QPixmap pixmap = QPixmap::fromImage(image.scaled(64, 64, Qt::KeepAspectRatio));
        QListWidgetItem *item = new QListWidgetItem(QIcon(pixmap), QString("Кадр %1").arg(i+1));
        item->setData(Qt::UserRole, i); // храним индекс
        frameList->addItem(item);
    }

    // Подключаем сигнал выбора элемента (можно переключать кадры по клику)
    connect(frameList, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        int index = item->data(Qt::UserRole).toInt();
        loadFrame(index);
    });
}

void MainWindow::loadFrame(int index)
{
    if (index < 0 || index >= m_frames.size())
        return;
    m_currentFrameIndex = index;
    m_canvasWidget->setPixels(m_frames[index]);
}

void MainWindow::onAddFrame()
{
    // Копируем текущие пиксели холста
    QVector<QVector<QColor>> currentPixels = m_canvasWidget->getPixelsCopy();
    m_frames.append(currentPixels);
    m_currentFrameIndex = m_frames.size() - 1;
    updateFrameLine();

    // Применяем эффект призрака (полупрозрачность) к текущему холсту
    m_canvasWidget->applyGhostEffect(0.5f);
}

void MainWindow::onClearFrame()
{
    // Очищаем холст (заполняем прозрачным)
    m_canvasWidget->setCanvasSize(m_canvasWidget->canvasSize().width(),
                                  m_canvasWidget->canvasSize().height());
}

void MainWindow::onPreviousFrame()
{
    if (m_frames.isEmpty())
        return;
    int newIndex = m_currentFrameIndex - 1;
    if (newIndex < 0)
        newIndex = m_frames.size() - 1;
    loadFrame(newIndex);
}

void MainWindow::onNextFrame()
{
    if (m_frames.isEmpty())
        return;
    int newIndex = m_currentFrameIndex + 1;
    if (newIndex >= m_frames.size())
        newIndex = 0;
    loadFrame(newIndex);
}

void MainWindow::updateStatusBar(const QColor &color)
{
    // Формируем строку вида "RGBA(255, 128, 64, 255)" или "#FF8040"
    QString colorText = QString("RGBA(%1, %2, %3, %4)")
                            .arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha());
    m_statusColorLabel->setText(colorText);

    // Создаём иконку цвета
    QPixmap pixmap(16, 16);
    pixmap.fill(color);
    m_statusColorIcon->setPixmap(pixmap);
}