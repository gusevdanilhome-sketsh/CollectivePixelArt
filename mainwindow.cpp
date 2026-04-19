#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "canvaswidget.h"
#include "colorpalette.h"
#include "spritewidget.h"
#include "navigationwidget.h"
#include "frame.h"
#include "layer.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QPushButton>
#include <QStatusBar>
#include <QPainter>
#include <QMessageBox>
#include <QButtonGroup>
#include <QMenuBar>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QBuffer>
#include <QInputDialog>
#include <QSlider>
#include <QLabel>
#include <QCheckBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    createMenuBar();

    // --- Холст ---
    QWidget *container = ui->ConvasWidget;
    if (!container) {
        container = new QWidget(ui->CanvasScrollArea);
        ui->CanvasScrollArea->setWidget(container);
    }

    m_canvasWidget = new CanvasWidget(container);
    m_canvasWidget->setCanvasSize(m_canvasSize);
    m_canvasWidget->setPixelSize(16);

    QHBoxLayout *canvasLayout = new QHBoxLayout(container);
    canvasLayout->addWidget(m_canvasWidget);
    canvasLayout->setAlignment(m_canvasWidget, Qt::AlignCenter);
    canvasLayout->setContentsMargins(0, 0, 0, 0);
    container->setLayout(canvasLayout);

    // Первый кадр и слой
    Frame *firstFrame = new Frame(m_canvasSize);
    firstFrame->addLayer(new Layer("Фон", m_canvasSize));
    m_frames.append(firstFrame);
    m_currentFrameIndex = 0;

    // --- Палитра ---
    QWidget *paletteContainer = ui->PaletteWidget;
    if (!paletteContainer) {
        paletteContainer = new QWidget(ui->ColorsTab);
        ui->gridLayout_3->addWidget(paletteContainer, 2, 0);
    }

    m_colorPalette = new ColorPalette(m_canvasWidget, paletteContainer);
    QVBoxLayout *paletteLayout = new QVBoxLayout(paletteContainer);
    paletteLayout->addWidget(m_colorPalette);
    paletteLayout->setContentsMargins(0, 0, 0, 0);
    paletteContainer->setLayout(paletteLayout);

    // Связываем спинбоксы из UI
    QSpinBox *sizeSpinBox = ui->BitDepthSpin;
    if (sizeSpinBox) {
        connect(sizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                m_colorPalette, &ColorPalette::setPaletteSize);
        m_colorPalette->setPaletteSize(sizeSpinBox->value());
    } else {
        m_colorPalette->setPaletteSize(16);
    }

    QSpinBox *alphaSpinBox = ui->AlfaChannelSpin;
    if (alphaSpinBox) {
        connect(alphaSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                m_canvasWidget, &CanvasWidget::setAlpha);
        m_canvasWidget->setAlpha(alphaSpinBox->value());
    } else {
        m_canvasWidget->setAlpha(255);
    }

    // --- Инструменты ---
    setupToolButtons();

    QSpinBox *brushSizeSpin = ui->BrushSizeSpin;
    if (brushSizeSpin) {
        connect(brushSizeSpin, QOverload<int>::of(&QSpinBox::valueChanged),
                m_canvasWidget, &CanvasWidget::setBrushSize);
        m_canvasWidget->setBrushSize(brushSizeSpin->value());
    }

    connect(m_canvasWidget, &CanvasWidget::colorPicked, this, [this](const QColor &color) {
        QSpinBox *alphaSpin = ui->AlfaChannelSpin;
        if (alphaSpin) {
            alphaSpin->setValue(color.alpha());
        }
    });

    // --- Навигация ---
    m_navigationWidget = new NavigationWidget(this);
    m_navigationWidget->setCanvasWidget(m_canvasWidget);
    // Помещаем в существующий ui->NavigationWidget
    QVBoxLayout *navLayout = new QVBoxLayout(ui->NavigationWidget);
    navLayout->addWidget(m_navigationWidget);
    navLayout->setContentsMargins(0, 0, 0, 0);
    ui->NavigationWidget->setLayout(navLayout);

    // --- Кнопки кадров ---
    connect(ui->AddFramesButton, &QPushButton::clicked, this, &MainWindow::onAddFrame);
    connect(ui->ClearFramesButton, &QPushButton::clicked, this, &MainWindow::onClearFrame);
    connect(ui->PrevFrameButton, &QPushButton::clicked, this, &MainWindow::onPreviousFrame);
    connect(ui->NextFrameButton, &QPushButton::clicked, this, &MainWindow::onNextFrame);

    // --- Анимация ---
    m_animationTimer = new QTimer(this);
    connect(m_animationTimer, &QTimer::timeout, this, &MainWindow::onAnimationTick);
    connect(ui->StattButton, &QPushButton::clicked, this, &MainWindow::onPlayAnimation);
    connect(ui->StopButton, &QPushButton::clicked, this, &MainWindow::onStopAnimation);

    QSpinBox *fpsSpinBox = ui->num_fps;
    if (fpsSpinBox) {
        connect(fpsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                this, &MainWindow::onFpsChanged);
    }

    connect(ui->ExportButton, &QPushButton::clicked, this, &MainWindow::onExportSpriteSheet);
    connect(ui->ImportButton, &QPushButton::clicked, this, &MainWindow::onLoadProject);
    connect(ui->DeletFrameButton, &QPushButton::clicked, [this]() {
        if (!m_frames.isEmpty() && m_currentFrameIndex >= 0) {
            delete m_frames.takeAt(m_currentFrameIndex);
            updateFrameLine();
            if (m_frames.isEmpty()) {
                Frame *newFrame = new Frame(m_canvasSize);
                newFrame->addLayer(new Layer("Фон", m_canvasSize));
                m_frames.append(newFrame);
                m_currentFrameIndex = 0;
            } else {
                m_currentFrameIndex = qMin(m_currentFrameIndex, m_frames.size() - 1);
                loadFrame(m_currentFrameIndex);
            }
            refreshCanvasFromFrame();
            m_spriteWidget->setFrame(currentFrame()->compositeImage());
        }
    });

    // --- Статус-бар ---
    createStatusBar();
    connect(m_canvasWidget, &CanvasWidget::currentColorChanged, this, &MainWindow::updateStatusBar);
    connect(m_canvasWidget, &CanvasWidget::toolChanged, this, &MainWindow::updateToolStatus);
    updateStatusBar(QColor(m_canvasWidget->currentColor().red(),
                           m_canvasWidget->currentColor().green(),
                           m_canvasWidget->currentColor().blue(),
                           m_canvasWidget->alpha()));
    updateToolStatus(m_canvasWidget->tool());

    // --- Панель слоёв (из UI) ---
    setupLayerPanel();

    // --- Кадровая дорожка ---
    setupFrameLine();

    // --- Спрайт ---
    setupSpriteView();

    // Инициализация
    refreshCanvasFromFrame();
    updateFrameLine();
    updateLayerList();

    ui->PrevFrameButton->setEnabled(false);
    ui->NextFrameButton->setEnabled(false);
}

MainWindow::~MainWindow()
{
    qDeleteAll(m_frames);
    delete ui;
}

void MainWindow::createMenuBar()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&Файл"));

    QAction *newProjectAct = fileMenu->addAction(tr("Новый проект..."));
    connect(newProjectAct, &QAction::triggered, this, &MainWindow::onNewProject);

    fileMenu->addSeparator();

    QAction *saveAct = fileMenu->addAction(tr("Сохранить проект..."));
    connect(saveAct, &QAction::triggered, this, &MainWindow::onSaveProject);

    QAction *loadAct = fileMenu->addAction(tr("Загрузить проект..."));
    connect(loadAct, &QAction::triggered, this, &MainWindow::onLoadProject);

    fileMenu->addSeparator();

    QAction *exportAtlasAct = fileMenu->addAction(tr("Экспорт атласа..."));
    connect(exportAtlasAct, &QAction::triggered, this, &MainWindow::onExportSpriteSheet);
}

void MainWindow::setupLayerPanel()
{
    // Создаём список слоёв внутри существующего ui->frame
    m_layerList = new QListWidget(ui->frame);
    m_layerList->setDragDropMode(QAbstractItemView::InternalMove);
    QVBoxLayout *frameLayout = new QVBoxLayout(ui->frame);
    frameLayout->addWidget(m_layerList);
    frameLayout->setContentsMargins(0, 0, 0, 0);
    ui->frame->setLayout(frameLayout);

    // Подключаем кнопки и контролы из UI
    connect(ui->pushButton_6, &QPushButton::clicked, this, &MainWindow::onNewLayer);
    connect(ui->pushButton_7, &QPushButton::clicked, this, &MainWindow::onDeleteLayer);
    connect(m_layerList, &QListWidget::currentRowChanged, this, &MainWindow::onLayerSelectionChanged);
    connect(ui->checkBox, &QCheckBox::toggled, this, &MainWindow::onLayerVisibilityChanged);
    connect(ui->horizontalSlider, &QSlider::valueChanged, this, &MainWindow::onLayerOpacityChanged);
}

void MainWindow::createStatusBar()
{
    m_statusColorIcon = new QLabel(this);
    m_statusColorIcon->setFixedSize(20, 20);
    m_statusColorLabel = new QLabel(this);
    m_statusToolLabel = new QLabel(this);

    statusBar()->addPermanentWidget(m_statusColorIcon);
    statusBar()->addPermanentWidget(m_statusColorLabel);
    statusBar()->addWidget(m_statusToolLabel);
}

void MainWindow::setupToolButtons()
{
    m_toolButtonGroup = new QButtonGroup(this);
    m_toolButtonGroup->setExclusive(true);

    m_toolButtonGroup->addButton(ui->BrushButton, CanvasWidget::Brush);
    m_toolButtonGroup->addButton(ui->EraserButton, CanvasWidget::Eraser);
    m_toolButtonGroup->addButton(ui->FillButton, CanvasWidget::Fill);
    m_toolButtonGroup->addButton(ui->PickerButton, CanvasWidget::Picker);
    m_toolButtonGroup->addButton(ui->RectangleButton, CanvasWidget::Rectangle);
    m_toolButtonGroup->addButton(ui->OvalButton, CanvasWidget::Ellipse);
    m_toolButtonGroup->addButton(ui->LineButton, CanvasWidget::Line);
    m_toolButtonGroup->addButton(ui->TriangleButton, CanvasWidget::Triangle);

    foreach (QAbstractButton *btn, m_toolButtonGroup->buttons()) {
        btn->setCheckable(true);
    }

    ui->BrushButton->setChecked(true);

    connect(m_toolButtonGroup, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &MainWindow::onToolButtonClicked);
}

void MainWindow::onToolButtonClicked(int id)
{
    CanvasWidget::Tool tool = static_cast<CanvasWidget::Tool>(id);
    m_canvasWidget->setTool(tool);
}

void MainWindow::setupFrameLine()
{
    QWidget *container = ui->TimeLineWifget;
    QVBoxLayout *layout = new QVBoxLayout(container);
    QListWidget *frameList = new QListWidget(container);
    frameList->setViewMode(QListWidget::IconMode);
    frameList->setIconSize(QSize(64, 64));
    frameList->setResizeMode(QListWidget::Adjust);
    frameList->setMovement(QListWidget::Static);
    frameList->setDragDropMode(QAbstractItemView::InternalMove);
    layout->addWidget(frameList);
    container->setLayout(layout);
    frameList->setObjectName("frameListWidget");

    connect(frameList, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        int index = item->data(Qt::UserRole).toInt();
        loadFrame(index);
        if (!m_animationTimer->isActive() && index >= 0 && index < m_frames.size()) {
            m_spriteWidget->setFrame(m_frames[index]->compositeImage());
        }
    });

    connect(frameList->model(), &QAbstractItemModel::rowsMoved, this, [this]() {
        QListWidget *list = ui->TimeLineWifget->findChild<QListWidget*>("frameListWidget");
        if (!list) return;
        QList<Frame*> newOrder;
        for (int i = 0; i < list->count(); ++i) {
            QListWidgetItem *item = list->item(i);
            int oldIndex = item->data(Qt::UserRole).toInt();
            newOrder.append(m_frames[oldIndex]);
        }
        m_frames = newOrder;
        for (int i = 0; i < list->count(); ++i) {
            list->item(i)->setData(Qt::UserRole, i);
        }
        if (m_currentFrameIndex != -1) {
            m_currentFrameIndex = list->currentRow();
        }
    });
}

void MainWindow::updateFrameLine()
{
    QListWidget *frameList = ui->TimeLineWifget->findChild<QListWidget*>("frameListWidget");
    if (!frameList) return;

    frameList->clear();
    for (int i = 0; i < m_frames.size(); ++i) {
        QImage img = m_frames[i]->compositeImage();
        QPixmap pixmap = QPixmap::fromImage(img.scaled(64, 64, Qt::KeepAspectRatio));
        QListWidgetItem *item = new QListWidgetItem(QIcon(pixmap), QString("Кадр %1").arg(i+1));
        item->setData(Qt::UserRole, i);
        frameList->addItem(item);
    }

    bool hasFrames = !m_frames.isEmpty();
    ui->PrevFrameButton->setEnabled(hasFrames);
    ui->NextFrameButton->setEnabled(hasFrames);
    ui->StattButton->setEnabled(hasFrames);
}

void MainWindow::loadFrame(int index)
{
    if (index < 0 || index >= m_frames.size())
        return;
    m_currentFrameIndex = index;
    refreshCanvasFromFrame();
}

void MainWindow::refreshCanvasFromFrame()
{
    Frame *frame = currentFrame();
    if (!frame) return;
    m_canvasWidget->setImage(frame->compositeImage());
    m_canvasWidget->clearGhostLayer();
    updateLayerList();
}

Frame* MainWindow::currentFrame() const
{
    if (m_currentFrameIndex >= 0 && m_currentFrameIndex < m_frames.size())
        return m_frames[m_currentFrameIndex];
    return nullptr;
}

Layer* MainWindow::currentLayer() const
{
    Frame *frame = currentFrame();
    if (!frame) return nullptr;
    int row = m_layerList->currentRow();
    if (row >= 0 && row < frame->layers().size())
        return frame->layers()[row];
    return nullptr;
}

void MainWindow::setupSpriteView()
{
    m_spriteWidget = new SpriteWidget(ui->SpriteWidget);
    QVBoxLayout *layout = new QVBoxLayout(ui->SpriteWidget);
    layout->addWidget(m_spriteWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    ui->SpriteWidget->setLayout(layout);
}

void MainWindow::onAddFrame()
{
    Frame *newFrame = new Frame(m_canvasSize);
    newFrame->addLayer(new Layer("Фон", m_canvasSize));
    m_frames.append(newFrame);
    m_currentFrameIndex = m_frames.size() - 1;
    updateFrameLine();
    refreshCanvasFromFrame();
    m_spriteWidget->setFrame(newFrame->compositeImage());
}

void MainWindow::onClearFrame()
{
    Frame *frame = currentFrame();
    if (frame) {
        for (Layer *layer : frame->layers()) {
            layer->setImage(QImage(m_canvasSize, QImage::Format_ARGB32));
            layer->image().fill(Qt::transparent);
        }
    }
    refreshCanvasFromFrame();
    m_canvasWidget->clearGhostLayer();
}

void MainWindow::onPreviousFrame()
{
    if (m_frames.isEmpty()) return;
    int newIndex = m_currentFrameIndex - 1;
    if (newIndex < 0) newIndex = m_frames.size() - 1;
    loadFrame(newIndex);
    m_spriteWidget->setFrame(currentFrame()->compositeImage());
}

void MainWindow::onNextFrame()
{
    if (m_frames.isEmpty()) return;
    int newIndex = m_currentFrameIndex + 1;
    if (newIndex >= m_frames.size()) newIndex = 0;
    loadFrame(newIndex);
    m_spriteWidget->setFrame(currentFrame()->compositeImage());
}

void MainWindow::onPlayAnimation()
{
    if (m_frames.size() < 2) {
        QMessageBox::information(this, tr("Анимация"), tr("Добавьте хотя бы два кадра"));
        return;
    }
    m_animationTimer->start(1000 / m_animationFps);
    if (m_currentFrameIndex >= 0 && m_currentFrameIndex < m_frames.size()) {
        m_spriteWidget->setFrame(currentFrame()->compositeImage());
    } else {
        m_currentFrameIndex = 0;
        m_spriteWidget->setFrame(m_frames[0]->compositeImage());
    }
}

void MainWindow::onStopAnimation()
{
    m_animationTimer->stop();
    if (m_currentFrameIndex >= 0 && m_currentFrameIndex < m_frames.size()) {
        m_spriteWidget->setFrame(currentFrame()->compositeImage());
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
    m_spriteWidget->setFrame(m_frames[newIndex]->compositeImage());
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

void MainWindow::updateToolStatus(CanvasWidget::Tool tool)
{
    QString toolName;
    switch (tool) {
    case CanvasWidget::Brush: toolName = tr("Кисть"); break;
    case CanvasWidget::Eraser: toolName = tr("Ластик"); break;
    case CanvasWidget::Fill: toolName = tr("Заливка"); break;
    case CanvasWidget::Picker: toolName = tr("Пипетка"); break;
    case CanvasWidget::Rectangle: toolName = tr("Прямоугольник"); break;
    case CanvasWidget::Ellipse: toolName = tr("Овал"); break;
    case CanvasWidget::Line: toolName = tr("Линия"); break;
    case CanvasWidget::Triangle: toolName = tr("Треугольник"); break;
    default: toolName = tr("Неизвестно");
    }
    m_statusToolLabel->setText(tr("Инструмент: %1").arg(toolName));
}

void MainWindow::onNewLayer()
{
    Frame *frame = currentFrame();
    if (!frame) return;
    bool ok;
    QString name = QInputDialog::getText(this, tr("Новый слой"), tr("Имя слоя:"), QLineEdit::Normal, tr("Слой %1").arg(frame->layers().size()+1), &ok);
    if (!ok) return;
    Layer *layer = new Layer(name, m_canvasSize);
    frame->addLayer(layer);
    updateLayerList();
    refreshCanvasFromFrame();
}

void MainWindow::onDeleteLayer()
{
    int row = m_layerList->currentRow();
    Frame *frame = currentFrame();
    if (!frame || row < 0 || row >= frame->layers().size()) return;
    if (frame->layers().size() <= 1) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Нельзя удалить последний слой."));
        return;
    }
    frame->removeLayer(row);
    updateLayerList();
    refreshCanvasFromFrame();
}

void MainWindow::onLayerSelectionChanged()
{
    Layer *layer = currentLayer();
    if (!layer) return;
    ui->checkBox->setChecked(layer->isVisible());
    ui->horizontalSlider->setValue(layer->opacity());
}

void MainWindow::onLayerVisibilityChanged(bool visible)
{
    Layer *layer = currentLayer();
    if (layer) {
        layer->setVisible(visible);
        refreshCanvasFromFrame();
    }
}

void MainWindow::onLayerOpacityChanged(int opacity)
{
    Layer *layer = currentLayer();
    if (layer) {
        layer->setOpacity(opacity);
        refreshCanvasFromFrame();
    }
}

void MainWindow::updateLayerList()
{
    Frame *frame = currentFrame();
    if (!frame) return;
    m_layerList->clear();
    for (Layer *layer : frame->layers()) {
        QListWidgetItem *item = new QListWidgetItem(layer->name());
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        m_layerList->addItem(item);
    }
    if (frame->layers().size() > 0) {
        m_layerList->setCurrentRow(0);
    }
}

void MainWindow::onNewProject()
{
    bool ok;
    int w = QInputDialog::getInt(this, tr("Новый проект"), tr("Ширина:"), 32, 1, 1024, 1, &ok);
    if (!ok) return;
    int h = QInputDialog::getInt(this, tr("Новый проект"), tr("Высота:"), 32, 1, 1024, 1, &ok);
    if (!ok) return;

    m_canvasSize = QSize(w, h);
    m_canvasWidget->setCanvasSize(m_canvasSize);

    qDeleteAll(m_frames);
    m_frames.clear();
    Frame *frame = new Frame(m_canvasSize);
    frame->addLayer(new Layer("Фон", m_canvasSize));
    m_frames.append(frame);
    m_currentFrameIndex = 0;
    refreshCanvasFromFrame();
    updateFrameLine();
    updateLayerList();
    m_spriteWidget->setFrame(frame->compositeImage());
}

void MainWindow::onSaveProject()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Сохранить проект"), QString(), tr("Pixel Art Project (*.pap)"));
    if (filename.isEmpty()) return;

    QJsonObject root;
    root["canvasSize"] = QJsonArray({m_canvasSize.width(), m_canvasSize.height()});
    QJsonArray framesArray;
    for (Frame *frame : m_frames) {
        QJsonObject frameObj;
        QJsonArray layersArray;
        for (Layer *layer : frame->layers()) {
            QJsonObject layerObj;
            layerObj["name"] = layer->name();
            layerObj["visible"] = layer->isVisible();
            layerObj["opacity"] = layer->opacity();
            QByteArray ba;
            QBuffer buffer(&ba);
            buffer.open(QIODevice::WriteOnly);
            layer->image().save(&buffer, "PNG");
            layerObj["data"] = QString(ba.toBase64());
            layersArray.append(layerObj);
        }
        frameObj["layers"] = layersArray;
        framesArray.append(frameObj);
    }
    root["frames"] = framesArray;

    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(root).toJson());
    }
}

void MainWindow::onLoadProject()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Загрузить проект"), QString(), tr("Pixel Art Project (*.pap)"));
    if (filename.isEmpty()) return;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) return;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject root = doc.object();
    QJsonArray sizeArr = root["canvasSize"].toArray();
    m_canvasSize = QSize(sizeArr[0].toInt(), sizeArr[1].toInt());
    m_canvasWidget->setCanvasSize(m_canvasSize);

    qDeleteAll(m_frames);
    m_frames.clear();
    QJsonArray framesArray = root["frames"].toArray();
    for (const QJsonValue &frameVal : framesArray) {
        QJsonObject frameObj = frameVal.toObject();
        Frame *frame = new Frame(m_canvasSize);
        QJsonArray layersArray = frameObj["layers"].toArray();
        for (const QJsonValue &layerVal : layersArray) {
            QJsonObject layerObj = layerVal.toObject();
            QString name = layerObj["name"].toString();
            bool visible = layerObj["visible"].toBool();
            int opacity = layerObj["opacity"].toInt();
            QByteArray ba = QByteArray::fromBase64(layerObj["data"].toString().toLatin1());
            QImage img;
            img.loadFromData(ba, "PNG");
            Layer *layer = new Layer(name, m_canvasSize);
            layer->setImage(img);
            layer->setVisible(visible);
            layer->setOpacity(opacity);
            frame->addLayer(layer);
        }
        m_frames.append(frame);
    }
    if (m_frames.isEmpty()) {
        Frame *frame = new Frame(m_canvasSize);
        frame->addLayer(new Layer("Фон", m_canvasSize));
        m_frames.append(frame);
    }
    m_currentFrameIndex = 0;
    refreshCanvasFromFrame();
    updateFrameLine();
    updateLayerList();
    m_spriteWidget->setFrame(currentFrame()->compositeImage());
}

void MainWindow::onExportSpriteSheet()
{
    if (m_frames.isEmpty()) return;
    QString filename = QFileDialog::getSaveFileName(this, tr("Экспорт атласа"), QString(), tr("PNG (*.png)"));
    if (filename.isEmpty()) return;

    int cols = qMin(10, m_frames.size());
    int rows = (m_frames.size() + cols - 1) / cols;
    QImage atlas(m_canvasSize.width() * cols, m_canvasSize.height() * rows, QImage::Format_ARGB32);
    atlas.fill(Qt::transparent);
    QPainter painter(&atlas);
    for (int i = 0; i < m_frames.size(); ++i) {
        int row = i / cols;
        int col = i % cols;
        painter.drawImage(col * m_canvasSize.width(), row * m_canvasSize.height(), m_frames[i]->compositeImage());
    }
    painter.end();
    atlas.save(filename);
}

void MainWindow::onCanvasChanged()
{
    Frame *frame = currentFrame();
    if (!frame) return;
    Layer *layer = currentLayer();
    if (layer) {
        layer->setImage(m_canvasWidget->image());
    }
    updateFrameLine();
    m_spriteWidget->setFrame(frame->compositeImage());
}