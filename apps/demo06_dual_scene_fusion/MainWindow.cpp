#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QScrollArea>
#include <QFrame>
#include <QSplitter>
#include <QFileInfo>
#include <QApplication>
#include <QCoreApplication>
#include <cmath>

// ─── 深色主题 QSS ──────────────────────────────────────────────────────────
static const char* DARK_QSS = R"(
QWidget { background:#1E1E2E; color:#E0E0E0; font-size:12px; }
QGroupBox { border:1px solid #3A3A5C; border-radius:4px; margin-top:8px; padding-top:6px; }
QGroupBox::title { subcontrol-origin:margin; left:8px; color:#8888BB; }
QPushButton { background:#2D2D4E; border:1px solid #4A4A7A; border-radius:3px;
              padding:4px 8px; min-height:26px; color:#C0C0E0; }
QPushButton:hover  { background:#3A3A6A; }
QPushButton:pressed{ background:#1A1A3A; }
QSlider::groove:horizontal { height:4px; background:#3A3A5C; border-radius:2px; }
QSlider::handle:horizontal { width:12px; height:12px; margin:-4px 0;
                              background:#5A5A9A; border-radius:6px; }
QDoubleSpinBox { background:#2A2A3E; border:1px solid #4A4A7A; border-radius:3px;
                 padding:2px 4px; min-height:22px; }
QCheckBox::indicator { width:14px; height:14px; border:1px solid #5A5A9A;
                       border-radius:2px; background:#2A2A3E; }
QCheckBox::indicator:checked { background:#5A5A9A; }
QComboBox { background:#2A2A3E; border:1px solid #4A4A7A; border-radius:3px;
            padding:2px 6px; min-height:22px; }
QComboBox QAbstractItemView { background:#2A2A3E; selection-background-color:#4A4A7A; }
QTextEdit { background:#161622; border:1px solid #3A3A5C; color:#A0A0C0; font-size:11px; }
QLabel { color:#C0C0D8; }
QScrollBar:vertical { width:8px; background:#1A1A2E; }
QScrollBar::handle:vertical { background:#3A3A5C; border-radius:4px; }
)";

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Demo-06: Dual Scene Fusion Viewer | LowAir-GS");
    resize(1600, 900);
    applyDarkTheme();
    buildUi();

    m_renderWidget->setLayerController(&m_layerCtrl);
    m_renderWidget->setFusionTransform(&m_fusionTransform);

    log("Demo-06 Dual Scene Fusion Viewer started.");
    log("Load a Mesh and a Gaussian to begin.");
}

void MainWindow::applyDarkTheme() {
    if (auto* app = qobject_cast<QApplication*>(QCoreApplication::instance()))
        app->setStyleSheet(DARK_QSS);
}

// ─── UI 构建 ──────────────────────────────────────────────────────────────
void MainWindow::buildUi() {
    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    QHBoxLayout* mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);

    // 左侧面板（固定宽度 260）
    QWidget* leftPanel = buildLeftPanel();
    leftPanel->setFixedWidth(260);

    // 中间渲染窗口
    m_renderWidget = new RenderWidget(this);
    connect(m_renderWidget, &RenderWidget::frameRendered,
            this, &MainWindow::onFrameRendered);

    // 右侧统计面板（固定宽度 240）
    QWidget* rightPanel = buildRightPanel();
    rightPanel->setFixedWidth(240);

    // 底部日志
    m_logEdit = new QTextEdit(this);
    m_logEdit->setReadOnly(true);
    m_logEdit->setFixedHeight(100);

    // 中间+日志垂直布局
    QWidget* centerWidget = new QWidget(this);
    QVBoxLayout* centerLayout = new QVBoxLayout(centerWidget);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(4);
    centerLayout->addWidget(m_renderWidget, 1);
    centerLayout->addWidget(m_logEdit, 0);

    mainLayout->addWidget(leftPanel,    0);
    mainLayout->addWidget(centerWidget, 1);
    mainLayout->addWidget(rightPanel,   0);
}

QWidget* MainWindow::buildLeftPanel() {
    QScrollArea* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget* panel = new QWidget();
    QVBoxLayout* vl = new QVBoxLayout(panel);
    vl->setContentsMargins(6, 6, 6, 6);
    vl->setSpacing(6);

    // ── 资产加载 ──
    QGroupBox* grpLoad = new QGroupBox("资产加载");
    QVBoxLayout* loadVl = new QVBoxLayout(grpLoad);
    m_lblMeshPath     = new QLabel("Mesh: (未加载)");
    m_lblMeshPath->setWordWrap(true);
    m_lblGaussianPath = new QLabel("Gaussian: (未加载)");
    m_lblGaussianPath->setWordWrap(true);
    QPushButton* btnOpenMesh = new QPushButton("Open Mesh");
    QPushButton* btnOpenGs   = new QPushButton("Open Gaussian");
    QPushButton* btnLoadCfg  = new QPushButton("Load Fusion Config");
    QPushButton* btnSaveCfg  = new QPushButton("Save Fusion Config");
    connect(btnOpenMesh, &QPushButton::clicked, this, &MainWindow::onOpenMesh);
    connect(btnOpenGs,   &QPushButton::clicked, this, &MainWindow::onOpenGaussian);
    connect(btnLoadCfg,  &QPushButton::clicked, this, &MainWindow::onLoadFusionConfig);
    connect(btnSaveCfg,  &QPushButton::clicked, this, &MainWindow::onSaveFusionConfig);
    loadVl->addWidget(m_lblMeshPath);
    loadVl->addWidget(btnOpenMesh);
    loadVl->addWidget(m_lblGaussianPath);
    loadVl->addWidget(btnOpenGs);
    loadVl->addWidget(btnLoadCfg);
    loadVl->addWidget(btnSaveCfg);

    // ── 显示模式 ──
    QGroupBox* grpMode = new QGroupBox("显示模式");
    QGridLayout* modeGl = new QGridLayout(grpMode);
    QPushButton* btnMeshOnly = new QPushButton("Mesh Only");
    QPushButton* btnGsOnly   = new QPushButton("Gaussian Only");
    QPushButton* btnDual     = new QPushButton("Dual Fusion");
    QPushButton* btnWire     = new QPushButton("Wireframe Cmp");
    connect(btnMeshOnly, &QPushButton::clicked, this, &MainWindow::onMeshOnly);
    connect(btnGsOnly,   &QPushButton::clicked, this, &MainWindow::onGaussianOnly);
    connect(btnDual,     &QPushButton::clicked, this, &MainWindow::onDualFusion);
    connect(btnWire,     &QPushButton::clicked, this, &MainWindow::onWireframeComparison);
    modeGl->addWidget(btnMeshOnly, 0, 0);
    modeGl->addWidget(btnGsOnly,   0, 1);
    modeGl->addWidget(btnDual,     1, 0);
    modeGl->addWidget(btnWire,     1, 1);

    // ── 图层控制 ──
    QGroupBox* grpLayer = new QGroupBox("图层控制");
    QVBoxLayout* layerVl = new QVBoxLayout(grpLayer);

    m_chkMeshEnabled = new QCheckBox("Mesh Enabled");
    m_chkMeshEnabled->setChecked(true);
    m_chkGsEnabled   = new QCheckBox("Gaussian Enabled");
    m_chkGsEnabled->setChecked(true);
    connect(m_chkMeshEnabled, &QCheckBox::toggled, [this](bool v){
        m_layerCtrl.setMeshEnabled(v);
    });
    connect(m_chkGsEnabled, &QCheckBox::toggled, [this](bool v){
        m_layerCtrl.setGaussianEnabled(v);
    });

    QLabel* lblMeshOp = new QLabel("Mesh Opacity:");
    m_sliderMeshOp = new QSlider(Qt::Horizontal);
    m_sliderMeshOp->setRange(0, 100);
    m_sliderMeshOp->setValue(100);
    connect(m_sliderMeshOp, &QSlider::valueChanged, this, &MainWindow::onMeshOpacityChanged);

    QLabel* lblGsOp = new QLabel("Gaussian Opacity:");
    m_sliderGsOp = new QSlider(Qt::Horizontal);
    m_sliderGsOp->setRange(0, 100);
    m_sliderGsOp->setValue(75);
    connect(m_sliderGsOp, &QSlider::valueChanged, this, &MainWindow::onGaussianOpacityChanged);

    QLabel* lblMeshMode = new QLabel("Mesh Mode:");
    m_cmbMeshMode = new QComboBox();
    m_cmbMeshMode->addItems({"Solid", "Wireframe", "Transparent"});
    connect(m_cmbMeshMode, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int idx){
        m_layerCtrl.setMeshWireframe(idx == 1);
    });

    m_chkGsSplat = new QCheckBox("Gaussian Splat Mode");
    m_chkGsSplat->setChecked(true);
    connect(m_chkGsSplat, &QCheckBox::toggled, [this](bool v){
        m_layerCtrl.setGaussianSplatMode(v);
    });

    m_chkMeshBbox = new QCheckBox("Show Mesh BBox");
    m_chkMeshBbox->setChecked(true);
    m_chkGsBbox   = new QCheckBox("Show Gaussian BBox");
    m_chkGsBbox->setChecked(true);
    m_chkGrid     = new QCheckBox("Show Grid");
    m_chkGrid->setChecked(true);
    m_chkAxis     = new QCheckBox("Show Axis");
    m_chkAxis->setChecked(true);
    connect(m_chkMeshBbox, &QCheckBox::toggled, [this](bool v){ m_layerCtrl.setShowMeshBbox(v); });
    connect(m_chkGsBbox,   &QCheckBox::toggled, [this](bool v){ m_layerCtrl.setShowGaussianBbox(v); });
    connect(m_chkGrid,     &QCheckBox::toggled, [this](bool v){ m_layerCtrl.setShowGrid(v); });
    connect(m_chkAxis,     &QCheckBox::toggled, [this](bool v){ m_layerCtrl.setShowAxis(v); });

    layerVl->addWidget(m_chkMeshEnabled);
    layerVl->addWidget(m_chkGsEnabled);
    layerVl->addWidget(lblMeshOp);
    layerVl->addWidget(m_sliderMeshOp);
    layerVl->addWidget(lblGsOp);
    layerVl->addWidget(m_sliderGsOp);
    layerVl->addWidget(lblMeshMode);
    layerVl->addWidget(m_cmbMeshMode);
    layerVl->addWidget(m_chkGsSplat);
    layerVl->addWidget(m_chkMeshBbox);
    layerVl->addWidget(m_chkGsBbox);
    layerVl->addWidget(m_chkGrid);
    layerVl->addWidget(m_chkAxis);

    // ── 对齐控制 ──
    QGroupBox* grpAlign = new QGroupBox("对齐控制（Gaussian→Mesh）");
    QGridLayout* alignGl = new QGridLayout(grpAlign);
    alignGl->setSpacing(3);

    auto makeSpn = [](double min, double max, double step) {
        QDoubleSpinBox* s = new QDoubleSpinBox();
        s->setRange(min, max);
        s->setSingleStep(step);
        s->setDecimals(3);
        return s;
    };
    m_spnTx    = makeSpn(-1000, 1000, 0.1);
    m_spnTy    = makeSpn(-1000, 1000, 0.1);
    m_spnTz    = makeSpn(-1000, 1000, 0.1);
    m_spnRx    = makeSpn(-180, 180, 1.0);
    m_spnRy    = makeSpn(-180, 180, 1.0);
    m_spnRz    = makeSpn(-180, 180, 1.0);
    m_spnScale = makeSpn(0.001, 1000, 0.01);
    m_spnScale->setValue(1.0);

    alignGl->addWidget(new QLabel("Tx:"), 0, 0); alignGl->addWidget(m_spnTx, 0, 1);
    alignGl->addWidget(new QLabel("Ty:"), 1, 0); alignGl->addWidget(m_spnTy, 1, 1);
    alignGl->addWidget(new QLabel("Tz:"), 2, 0); alignGl->addWidget(m_spnTz, 2, 1);
    alignGl->addWidget(new QLabel("Rx°:"), 3, 0); alignGl->addWidget(m_spnRx, 3, 1);
    alignGl->addWidget(new QLabel("Ry°:"), 4, 0); alignGl->addWidget(m_spnRy, 4, 1);
    alignGl->addWidget(new QLabel("Rz°:"), 5, 0); alignGl->addWidget(m_spnRz, 5, 1);
    alignGl->addWidget(new QLabel("Scale:"), 6, 0); alignGl->addWidget(m_spnScale, 6, 1);

    QPushButton* btnApply      = new QPushButton("Apply Transform");
    QPushButton* btnReset      = new QPushButton("Reset Transform");
    QPushButton* btnCenter     = new QPushButton("Center Align");
    QPushButton* btnBboxScale  = new QPushButton("BBox Scale Match");
    QPushButton* btnFit        = new QPushButton("Fit To Scene");
    QPushButton* btnScreenshot = new QPushButton("Take Screenshot");
    connect(btnApply,      &QPushButton::clicked, this, &MainWindow::onApplyTransform);
    connect(btnReset,      &QPushButton::clicked, this, &MainWindow::onResetAlignment);
    connect(btnCenter,     &QPushButton::clicked, this, &MainWindow::onCenterAlign);
    connect(btnBboxScale,  &QPushButton::clicked, this, &MainWindow::onBboxScaleMatch);
    connect(btnFit,        &QPushButton::clicked, this, &MainWindow::onFitToScene);
    connect(btnScreenshot, &QPushButton::clicked, this, &MainWindow::onTakeScreenshot);
    alignGl->addWidget(btnApply,      7, 0, 1, 2);
    alignGl->addWidget(btnReset,      8, 0, 1, 2);
    alignGl->addWidget(btnCenter,     9, 0, 1, 2);
    alignGl->addWidget(btnBboxScale, 10, 0, 1, 2);
    alignGl->addWidget(btnFit,       11, 0, 1, 2);
    alignGl->addWidget(btnScreenshot,12, 0, 1, 2);

    vl->addWidget(grpLoad);
    vl->addWidget(grpMode);
    vl->addWidget(grpLayer);
    vl->addWidget(grpAlign);
    vl->addStretch();

    scroll->setWidget(panel);
    return scroll;
}

QWidget* MainWindow::buildRightPanel() {
    QScrollArea* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget* panel = new QWidget();
    QVBoxLayout* vl = new QVBoxLayout(panel);
    vl->setContentsMargins(6, 6, 6, 6);
    vl->setSpacing(6);

    m_lblFusionMode = new QLabel("Mode: Dual Fusion");
    m_lblFusionMode->setStyleSheet("color:#88BBFF; font-weight:bold; font-size:13px;");

    QGroupBox* grpMesh = new QGroupBox("Mesh 统计");
    QVBoxLayout* meshVl = new QVBoxLayout(grpMesh);
    m_lblMeshStats = new QLabel("(未加载)");
    m_lblMeshStats->setWordWrap(true);
    meshVl->addWidget(m_lblMeshStats);

    QGroupBox* grpGs = new QGroupBox("Gaussian 统计");
    QVBoxLayout* gsVl = new QVBoxLayout(grpGs);
    m_lblGsStats = new QLabel("(未加载)");
    m_lblGsStats->setWordWrap(true);
    gsVl->addWidget(m_lblGsStats);

    QGroupBox* grpAlign = new QGroupBox("对齐偏差");
    QVBoxLayout* alignVl = new QVBoxLayout(grpAlign);
    m_lblAlignStats = new QLabel("(待加载双源)");
    m_lblAlignStats->setWordWrap(true);
    alignVl->addWidget(m_lblAlignStats);

    QGroupBox* grpPerf = new QGroupBox("性能统计");
    QVBoxLayout* perfVl = new QVBoxLayout(grpPerf);
    m_lblPerfStats = new QLabel("FPS: --\n帧耗时: -- ms");
    m_lblPerfStats->setWordWrap(true);
    perfVl->addWidget(m_lblPerfStats);

    vl->addWidget(m_lblFusionMode);
    vl->addWidget(grpMesh);
    vl->addWidget(grpGs);
    vl->addWidget(grpAlign);
    vl->addWidget(grpPerf);
    vl->addStretch();

    scroll->setWidget(panel);
    return scroll;
}

// ─── 槽函数 ──────────────────────────────────────────────────────────────
void MainWindow::onOpenMesh() {
    QString path = QFileDialog::getOpenFileName(
        this, "Open Mesh", QString(),
        "Mesh Files (*.obj *.ply *.glb *.gltf *.fbx *.dae);;All Files (*)");
    if (path.isEmpty()) return;

    log("Loading Mesh: " + path);
    if (!m_meshLoader.load(path.toStdString(), m_meshAsset)) {
        log("ERROR: " + QString::fromStdString(m_meshLoader.lastError()));
        QMessageBox::warning(this, "Load Error", QString::fromStdString(m_meshLoader.lastError()));
        return;
    }
    m_lblMeshPath->setText("Mesh: " + QFileInfo(path).fileName());
    m_renderWidget->uploadMesh(m_meshAsset);
    m_metrics.meshVertices  = m_meshAsset.totalVertices;
    m_metrics.meshTriangles = m_meshAsset.totalTriangles;
    m_metrics.meshBboxMin   = m_meshAsset.bboxMin;
    m_metrics.meshBboxMax   = m_meshAsset.bboxMax;
    m_metrics.meshCenter    = m_meshAsset.center();
    m_metrics.meshSize      = m_meshAsset.size();
    m_metrics.computeAlignment();
    updateStatsPanel();
    log(QString("Mesh loaded: %1 vertices, %2 triangles")
        .arg(m_meshAsset.totalVertices).arg(m_meshAsset.totalTriangles));
}

void MainWindow::onOpenGaussian() {
    QString path = QFileDialog::getOpenFileName(
        this, "Open Gaussian", QString(),
        "Gaussian Files (*.ply *.splat);;All Files (*)");
    if (path.isEmpty()) return;

    log("Loading Gaussian: " + path);
    QString gsError;
    if (!GaussianPlyLoader::load(path, m_gaussianCloud, gsError)) {
        log("ERROR: " + gsError);
        QMessageBox::warning(this, "Load Error", gsError);
        return;
    }
    m_lblGaussianPath->setText("GS: " + QFileInfo(path).fileName());
    m_renderWidget->uploadGaussian(m_gaussianCloud);
    m_metrics.gaussianCount    = m_gaussianCloud.count();
    m_metrics.gaussianBboxMin  = m_gaussianCloud.bboxMin();
    m_metrics.gaussianBboxMax  = m_gaussianCloud.bboxMax();
    m_metrics.gaussianCenter   = m_gaussianCloud.center();
    m_metrics.gaussianSize     = m_gaussianCloud.bboxSize();
    m_metrics.computeAlignment();
    updateStatsPanel();
    log(QString("Gaussian loaded: %1 points").arg(m_metrics.gaussianCount));
}

void MainWindow::onLoadFusionConfig() {
    QString path = QFileDialog::getOpenFileName(
        this, "Load Fusion Config", QString(), "JSON (*.json);;All Files (*)");
    if (path.isEmpty()) return;
    bool ok = false;
    FusionConfig cfg = FusionConfig::loadFromFile(path, &ok);
    if (!ok) {
        log("[ERROR] Failed to load fusion config: " + path);
        return;
    }
    // 将对齐参数应用到当前变换
    m_fusionTransform.setTranslation(cfg.alignTranslation);
    m_fusionTransform.setRotationDeg(cfg.alignRotationDeg);
    m_fusionTransform.setScale(cfg.alignScale);
    updateTransformSpinboxes();
    log("Fusion config loaded: " + path);
}

void MainWindow::onSaveFusionConfig() {
    QString path = QFileDialog::getSaveFileName(
        this, "Save Fusion Config", "demo06_fusion_config.json", "JSON (*.json)");
    if (path.isEmpty()) return;
    // 将当前对齐参数写入配置
    FusionConfig cfg = FusionConfig::defaultConfig();
    cfg.alignTranslation = m_fusionTransform.translation();
    cfg.alignRotationDeg = m_fusionTransform.rotationDeg();
    cfg.alignScale       = m_fusionTransform.scale();
    cfg.displayMode      = m_metrics.displayMode;
    if (cfg.saveToFile(path)) {
        log("Fusion config saved: " + path);
    } else {
        log("[ERROR] Failed to save fusion config: " + path);
    }
}

void MainWindow::onMeshOnly() {
    m_layerCtrl.setDisplayMode(FusionDisplayMode::MeshOnly);
    m_lblFusionMode->setText("Mode: Mesh Only");
    m_metrics.displayMode = "Mesh Only";
    updateStatsPanel();
    log("Display mode: Mesh Only");
}

void MainWindow::onGaussianOnly() {
    m_layerCtrl.setDisplayMode(FusionDisplayMode::GaussianOnly);
    m_lblFusionMode->setText("Mode: Gaussian Only");
    m_metrics.displayMode = "Gaussian Only";
    updateStatsPanel();
    log("Display mode: Gaussian Only");
}

void MainWindow::onDualFusion() {
    m_layerCtrl.setDisplayMode(FusionDisplayMode::DualFusion);
    m_lblFusionMode->setText("Mode: Dual Fusion");
    m_metrics.displayMode = "Dual Fusion";
    updateStatsPanel();
    log("Display mode: Dual Fusion");
}

void MainWindow::onWireframeComparison() {
    m_layerCtrl.setDisplayMode(FusionDisplayMode::WireframeComparison);
    m_lblFusionMode->setText("Mode: Wireframe Comparison");
    m_metrics.displayMode = "Wireframe Comparison";
    updateStatsPanel();
    log("Display mode: Wireframe Comparison");
}

void MainWindow::onResetAlignment() {
    m_fusionTransform.reset();
    updateTransformSpinboxes();
    log("Alignment reset to identity.");
}

void MainWindow::onCenterAlign() {
    if (!m_meshAsset.isLoaded() || m_gaussianCloud.count() == 0) {
        log("Need both Mesh and Gaussian loaded for Center Align.");
        return;
    }
    m_fusionTransform.centerAlign(m_meshAsset.center(), m_gaussianCloud.center());
    updateTransformSpinboxes();
    log(QString("Center align applied. Mesh center: (%.2f, %.2f, %.2f)")
        .arg(m_meshAsset.center().x())
        .arg(m_meshAsset.center().y())
        .arg(m_meshAsset.center().z()));
}

void MainWindow::onFitToScene() {
    QVector3D center = m_meshAsset.isLoaded() ? m_meshAsset.center() : QVector3D(0,0,0);
    QVector3D sz     = m_meshAsset.isLoaded() ? m_meshAsset.size()   : QVector3D(10,10,10);
    float radius = std::max({sz.x(), sz.y(), sz.z()}) * 0.5f;
    m_renderWidget->fitToScene(center, radius);
    log("Camera fit to scene.");
}

void MainWindow::onBboxScaleMatch() {
    if (!m_meshAsset.isLoaded() || m_gaussianCloud.count() == 0) {
        log("Need both Mesh and Gaussian loaded for BBox Scale Match.");
        return;
    }
    m_fusionTransform.bboxScaleMatch(m_meshAsset.size(), m_gaussianCloud.bboxSize());
    updateTransformSpinboxes();
    log(QString("BBox scale match applied. Scale: %.4f").arg(m_fusionTransform.scale()));
}

void MainWindow::onApplyTransform() {
    m_fusionTransform.setTranslation({
        (float)m_spnTx->value(),
        (float)m_spnTy->value(),
        (float)m_spnTz->value()
    });
    m_fusionTransform.setRotationDeg({
        (float)m_spnRx->value(),
        (float)m_spnRy->value(),
        (float)m_spnRz->value()
    });
    m_fusionTransform.setScale((float)m_spnScale->value());
    log("Transform applied.");
}

void MainWindow::onTakeScreenshot() {
    QString path = QFileDialog::getSaveFileName(
        this, "Save Screenshot",
        QString("demo06_screenshot_%1.png")
            .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
        "PNG (*.png)");
    if (path.isEmpty()) return;
    m_renderWidget->takeScreenshot(path);
    log("Screenshot saved: " + path);
}

void MainWindow::onFrameRendered(float fps, float frameMs) {
    m_metrics.fps         = fps;
    m_metrics.frameTimeMs = frameMs;
    m_metrics.meshOpacity     = m_layerCtrl.meshOpacity();
    m_metrics.gaussianOpacity = m_layerCtrl.gaussianOpacity();
    m_lblPerfStats->setText(
        QString("FPS: %1\n帧耗时: %2 ms\nMesh Opacity: %3\nGS Opacity: %4\nMode: %5")
        .arg(fps, 0, 'f', 1)
        .arg(frameMs, 0, 'f', 2)
        .arg(m_layerCtrl.meshOpacity(), 0, 'f', 2)
        .arg(m_layerCtrl.gaussianOpacity(), 0, 'f', 2)
        .arg(QString::fromStdString(m_layerCtrl.displayModeName()))
    );
}

void MainWindow::onMeshOpacityChanged(int val) {
    m_layerCtrl.setMeshOpacity(val / 100.0f);
}

void MainWindow::onGaussianOpacityChanged(int val) {
    m_layerCtrl.setGaussianOpacity(val / 100.0f);
}

void MainWindow::updateStatsPanel() {
    if (m_meshAsset.isLoaded()) {
        m_lblMeshStats->setText(
            QString("顶点数: %1\n三角面: %2\nBBox Min: (%.2f,%.2f,%.2f)\nBBox Max: (%.2f,%.2f,%.2f)\n中心: (%.2f,%.2f,%.2f)")
            .arg(m_metrics.meshVertices)
            .arg(m_metrics.meshTriangles)
            .arg(m_metrics.meshBboxMin.x()).arg(m_metrics.meshBboxMin.y()).arg(m_metrics.meshBboxMin.z())
            .arg(m_metrics.meshBboxMax.x()).arg(m_metrics.meshBboxMax.y()).arg(m_metrics.meshBboxMax.z())
            .arg(m_metrics.meshCenter.x()).arg(m_metrics.meshCenter.y()).arg(m_metrics.meshCenter.z())
        );
    }
    if (m_gaussianCloud.count() > 0) {
        m_lblGsStats->setText(
            QString("Gaussian 数: %1\nBBox Min: (%.2f,%.2f,%.2f)\nBBox Max: (%.2f,%.2f,%.2f)\n中心: (%.2f,%.2f,%.2f)")
            .arg(m_metrics.gaussianCount)
            .arg(m_metrics.gaussianBboxMin.x()).arg(m_metrics.gaussianBboxMin.y()).arg(m_metrics.gaussianBboxMin.z())
            .arg(m_metrics.gaussianBboxMax.x()).arg(m_metrics.gaussianBboxMax.y()).arg(m_metrics.gaussianBboxMax.z())
            .arg(m_metrics.gaussianCenter.x()).arg(m_metrics.gaussianCenter.y()).arg(m_metrics.gaussianCenter.z())
        );
    }
    if (m_meshAsset.isLoaded() && m_gaussianCloud.count() > 0) {
        m_lblAlignStats->setText(
            QString("中心偏移: (%.3f,%.3f,%.3f)\nBBox比例 X: %.3f\nBBox比例 Y: %.3f\nBBox比例 Z: %.3f")
            .arg(m_metrics.centerOffset.x())
            .arg(m_metrics.centerOffset.y())
            .arg(m_metrics.centerOffset.z())
            .arg(m_metrics.bboxSizeRatioX)
            .arg(m_metrics.bboxSizeRatioY)
            .arg(m_metrics.bboxSizeRatioZ)
        );
    }
}

void MainWindow::updateTransformSpinboxes() {
    m_spnTx->setValue(m_fusionTransform.translation().x());
    m_spnTy->setValue(m_fusionTransform.translation().y());
    m_spnTz->setValue(m_fusionTransform.translation().z());
    m_spnRx->setValue(m_fusionTransform.rotationDeg().x());
    m_spnRy->setValue(m_fusionTransform.rotationDeg().y());
    m_spnRz->setValue(m_fusionTransform.rotationDeg().z());
    m_spnScale->setValue(m_fusionTransform.scale());
}

void MainWindow::log(const QString& msg) {
    QString ts = QDateTime::currentDateTime().toString("hh:mm:ss");
    m_logEdit->append(QString("[%1] %2").arg(ts, msg));
}
