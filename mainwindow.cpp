#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "canvaswidget.h"
#include "colorpalette.h"
#include "spritewidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QPushButton>
#include <QListWidget>
#include <QStatusBar>
#include <QPainter>
#include <QMessageBox>
#include <QButtonGroup>

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

    QSpinBox *sizeSpinBox = ui->palettesize;
    if (sizeSpinBox) {
        sizeSpinBox->setRange(1, 32);
        connect(sizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                m_colorPalette, &ColorPalette::setPaletteSize);
        m_colorPalette->setPaletteSize(sizeSpinBox->value());
    } else {
        m_colorPalette->setPaletteSize(16);
    }

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

    // --- Инструменты рисования ---
    QButtonGroup *toolGroup = new QButtonGroup(this);
    toolGroup->addButton(ui->toolBrush, static_cast<int>(CanvasWidget::Brush));
    toolGroup->addButton(ui->toolEraser, static_cast<int>(CanvasWidget::Eraser));
    toolGroup->addButton(ui->toolFill, static_cast<int>(CanvasWidget::Fill));
    toolGroup->addButton(ui->toolPicker, static_cast<int>(CanvasWidget::Picker));

    connect(toolGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),
            this, [this](QAbstractButton *button) {
                int id = button->group()->id(button);
                m_canvasWidget->setTool(static_cast<CanvasWidget::Tool>(id));
            });

    QSpinBox *brushSizeSpin = ui->brushSizeSpinBox;
    if (brushSizeSpin) {
        brushSizeSpin->setRange(1, 10);
        brushSizeSpin->setValue(1);
        connect(brushSizeSpin, QOverload<int>::of(&QSpinBox::valueChanged),
                m_canvasWidget, &CanvasWidget::setBrushSize);
        m_canvasWidget->setBrushSize(brushSizeSpin->value());
    }

    connect(m_canvasWidget, &CanvasWidget::colorPicked, this, [this](const QColor &color) {
        QSpinBox *alphaSpin = ui->spinBox;
        if (alphaSpin) {
            alphaSpin->setValue(color.alpha());
        }
    });

    // --- Кнопки управления кадрами ---
    connect(ui->addframes, &QPushButton::clicked, this, &MainWindow::onAddFrame);
    connect(ui->clearframes, &QPushButton::clicked, this, &MainWindow::onClearFrame);
    connect(ui->prevframe, &QPushButton::clicked, this, &MainWindow::onPreviousFrame);
    connect(ui->nextframe, &QPushButton::clicked, this, &MainWindow::onNextFrame);

    // --- Анимация ---
    m_animationTimer = new QTimer(this);
    connect(m_animationTimer, &QTimer::timeout, this, &MainWindow::onAnimationTick);
    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::onPlayAnimation);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &MainWindow::onStopAnimation);

    QSpinBox *fpsSpinBox = ui->num_fps;
    if (fpsSpinBox) {
        fpsSpinBox->setRange(1, 120);
        fpsSpinBox->setValue(m_animationFps);
        connect(fpsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                this, &MainWindow::onFpsChanged);
    }

    ui->pushButton_3->setText(tr("Экспорт"));
    ui->pushButton_4->setText(tr("Импорт"));
    ui->pushButton_5->setText(tr("Удалить кадр"));
    ui->label->setText(tr("Готов"));

    connect(ui->pushButton_3, &QPushButton::clicked, []() {
        QMessageBox::information(nullptr, "Экспорт", "Функция в разработке");
    });
    connect(ui->pushButton_4, &QPushButton::clicked, []() {
        QMessageBox::information(nullptr, "Импорт", "Функция в разработке");
    });
    connect(ui->pushButton_5, &QPushButton::clicked, [this]() {
        if (!m_frames.isEmpty() && m_currentFrameIndex >= 0) {
            m_frames.removeAt(m_currentFrameIndex);
            updateFrameLine();
            if (m_frames.isEmpty()) {
                m_canvasWidget->setCanvasSize(m_canvasWidget->canvasSize().width(),
                                              m_canvasWidget->canvasSize().height());
                m_currentFrameIndex = -1;
                m_spriteWidget->clear();
            } else {
                m_currentFrameIndex = qMin(m_currentFrameIndex, m_frames.size() - 1);
                loadFrame(m_currentFrameIndex);
            }
        }
    });

    // --- Статус-бар ---
    m_statusColorLabel = new QLabel(this);
    m_statusColorIcon = new QLabel(this);
    m_statusColorIcon->setFixedSize(20, 20);
    statusBar()->addPermanentWidget(m_statusColorIcon);
    statusBar()->addPermanentWidget(m_statusColorLabel);

    connect(m_canvasWidget, &CanvasWidget::currentColorChanged, this, &MainWindow::updateStatusBar);
    updateStatusBar(QColor(m_canvasWidget->currentColor().red(),
                           m_canvasWidget->currentColor().green(),
                           m_canvasWidget->currentColor().blue(),
                           m_canvasWidget->alpha()));

    // --- Кадровая дорожка ---
    setupFrameLine();

    // --- Спрайт (предпросмотр анимации) ---
    setupSpriteView();

    ui->prevframe->setEnabled(false);
    ui->nextframe->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupFrameLine()
{
    QWidget *container = ui->widget;
    QVBoxLayout *layout = new QVBoxLayout(container);
    QListWidget *frameList = new QListWidget(container);
    frameList->setViewMode(QListWidget::IconMode);
    frameList->setIconSize(QSize(64, 64));
    frameList->setResizeMode(QListWidget::Adjust);
    frameList->setMovement(QListWidget::Static);
    layout->addWidget(frameList);
    container->setLayout(layout);
    frameList->setObjectName("frameListWidget");

    connect(frameList, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        int index = item->data(Qt::UserRole).toInt();
        loadFrame(index);
        if (!m_animationTimer->isActive() && index >= 0 && index < m_frames.size()) {
            m_spriteWidget->setFrame(m_frames[index]);
        }
    });
}

void MainWindow::updateFrameLine()
{
    QListWidget *frameList = ui->widget->findChild<QListWidget*>("frameListWidget");
    if (!frameList) return;

    frameList->clear();
    for (int i = 0; i < m_frames.size(); ++i) {
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
        item->setData(Qt::UserRole, i);
        frameList->addItem(item);
    }

    bool hasFrames = !m_frames.isEmpty();
    ui->prevframe->setEnabled(hasFrames);
    ui->nextframe->setEnabled(hasFrames);
    ui->pushButton->setEnabled(hasFrames);
}

void MainWindow::loadFrame(int index)
{
    if (index < 0 || index >= m_frames.size())
        return;
    m_currentFrameIndex = index;
    m_canvasWidget->setPixels(m_frames[index]);
    m_canvasWidget->clearGhostLayer();
}

void MainWindow::setupSpriteView()
{
    QWidget *spriteTab = ui->sprite;
    QVBoxLayout *layout = new QVBoxLayout(spriteTab);
    m_spriteWidget = new SpriteWidget(spriteTab);
    layout->addWidget(m_spriteWidget);
    spriteTab->setLayout(layout);
}

void MainWindow::onAddFrame()
{
    QVector<QVector<QColor>> currentPixels = m_canvasWidget->getPixelsCopy();
    m_frames.append(currentPixels);
    m_currentFrameIndex = m_frames.size() - 1;
    updateFrameLine();

    QVector<QVector<QColor>> ghost = m_canvasWidget->createGhostFromCurrent(0.5f);
    m_canvasWidget->setCanvasSize(m_canvasWidget->canvasSize().width(),
                                  m_canvasWidget->canvasSize().height());
    m_canvasWidget->setGhostLayer(ghost);

    m_spriteWidget->setFrame(currentPixels);
}

void MainWindow::onClearFrame()
{
    m_canvasWidget->setCanvasSize(m_canvasWidget->canvasSize().width(),
                                  m_canvasWidget->canvasSize().height());
    m_canvasWidget->clearGhostLayer();
}

void MainWindow::onPreviousFrame()
{
    if (m_frames.isEmpty()) return;
    int newIndex = m_currentFrameIndex - 1;
    if (newIndex < 0) newIndex = m_frames.size() - 1;
    loadFrame(newIndex);
    m_spriteWidget->setFrame(m_frames[newIndex]);
}

void MainWindow::onNextFrame()
{
    if (m_frames.isEmpty()) return;
    int newIndex = m_currentFrameIndex + 1;
    if (newIndex >= m_frames.size()) newIndex = 0;
    loadFrame(newIndex);
    m_spriteWidget->setFrame(m_frames[newIndex]);
}

void MainWindow::onPlayAnimation()
{
    if (m_frames.size() < 2) {
        QMessageBox::information(this, tr("Анимация"), tr("Добавьте хотя бы два кадра"));
        return;
    }
    m_animationTimer->start(1000 / m_animationFps);
    if (m_currentFrameIndex >= 0 && m_currentFrameIndex < m_frames.size()) {
        m_spriteWidget->setFrame(m_frames[m_currentFrameIndex]);
    } else {
        m_currentFrameIndex = 0;
        m_spriteWidget->setFrame(m_frames[0]);
    }
}

void MainWindow::onStopAnimation()
{
    m_animationTimer->stop();
    if (m_currentFrameIndex >= 0 && m_currentFrameIndex < m_frames.size()) {
        m_spriteWidget->setFrame(m_frames[m_currentFrameIndex]);
    }
}

void MainWindow::onAnimationTick()
{
    if (m_frames.isEmpty()) {
        m_animationTimer->stop();
        return;
    }
    int newIndex = (m_currentFrameIndex + 1) % m_frames.size();
    m_currentFrameIndex = newIndex;
    m_spriteWidget->setFrame(m_frames[newIndex]);
}

void MainWindow::onFpsChanged(int fps)
{
    m_animationFps = fps;
    if (m_animationTimer->isActive()) {
        m_animationTimer->start(1000 / m_animationFps);
    }
}

void MainWindow::updateStatusBar(const QColor &color)
{
    QString colorText = QString("RGBA(%1, %2, %3, %4)")
    .arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha());
    m_statusColorLabel->setText(colorText);

    QPixmap pixmap(16, 16);
    pixmap.fill(color);
    m_statusColorIcon->setPixmap(pixmap);
}