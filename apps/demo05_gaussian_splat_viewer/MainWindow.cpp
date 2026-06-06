#include "MainWindow.h"
#include "gaussian/GaussianPlyLoader.h"
#include "gaussian/GaussianSplatLoader.h"
#include <QApplication>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QSplitter>
#include <QScrollArea>
#include <QDateTime>
#include <QMessageBox>
#include <QFileInfo>
#include <QDebug>

static const QString DARK_STYLE = R"(
QMainWindow, QWidget { background: #1E1E2E; color: #E0E0E0; }
QGroupBox { border: 1px solid #3A3A4A; border-radius: 4px; margin-top: 8px; padding-top: 4px; font-weight: bold; color: #A0C4FF; }
QGroupBox::title { subcontrol-origin: margin; left: 8px; padding: 0 4px; }
QPushButton { background: #2D2D42; border: 1px solid #4A4A6A; border-radius: 4px; padding: 4px 10px; color: #E0E0E0; min-height: 24px; }
QPushButton:hover { background: #3A3A58; }
QPushButton:pressed { background: #252538; }
QComboBox { background: #2D2D42; border: 1px solid #4A4A6A; border-radius: 4px; padding: 2px 6px; color: #E0E0E0; min-height: 24px; }
QComboBox QAbstractItemView { background: #2D2D42; color: #E0E0E0; selection-background-color: #4A4A6A; }
QCheckBox { color: #E0E0E0; }
QCheckBox::indicator { width: 14px; height: 14px; }
QDoubleSpinBox { background: #2D2D42; border: 1px solid #4A4A6A; border-radius: 4px; padding: 2px 4px; color: #E0E0E0; min-height: 22px; }
QLabel { color: #C0C0D0; }
QLabel#value { color: #A0E0A0; }
QTextEdit { background: #141420; border: 1px solid #3A3A4A; color: #90D090; font-family: monospace; font-size: 11px; }
QScrollArea { border: none; }
QSplitter::handle { background: #3A3A4A; }
)";

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("LowAir-GS | Demo-05 Gaussian Splat Viewer");
    resize(1400, 860);
    setStyleSheet(DARK_STYLE);
    buildUi();
}

void MainWindow::buildUi() {
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QHBoxLayout *mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(6, 6, 6, 6);
    mainLayout->setSpacing(6);

    // ===== 左侧控制面板 =====
    QWidget *leftPanel = new QWidget;
    leftPanel->setFixedWidth(240);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(4, 4, 4, 4);
    leftLayout->setSpacing(6);

    // 文件操作组
    QGroupBox *grpFile = new QGroupBox("Gaussian File");
    QVBoxLayout *fileLayout = new QVBoxLayout(grpFile);
    m_btnOpen = new QPushButton("Open Gaussian PLY");
    m_btnReload = new QPushButton("Reload");
    m_btnReload->setEnabled(false);
    fileLayout->addWidget(m_btnOpen);
    fileLayout->addWidget(m_btnReload);

    // 视图控制组
    QGroupBox *grpView = new QGroupBox("View");
    QVBoxLayout *viewLayout = new QVBoxLayout(grpView);
    m_btnResetView = new QPushButton("Reset View");
    m_btnFitToCloud = new QPushButton("Fit To Cloud");
    m_btnScreenshot = new QPushButton("Take Screenshot");
    viewLayout->addWidget(m_btnResetView);
    viewLayout->addWidget(m_btnFitToCloud);
    viewLayout->addWidget(m_btnScreenshot);

    // 渲染模式组
    QGroupBox *grpMode = new QGroupBox("Render Mode");
    QVBoxLayout *modeLayout = new QVBoxLayout(grpMode);
    m_cmbRenderMode = new QComboBox;
    m_cmbRenderMode->addItems({"Splat Mode", "Point Mode", "Debug Color Mode"});
    m_chkGrid = new QCheckBox("Show Grid");
    m_chkAxis = new QCheckBox("Show Axis");
    m_chkBBox = new QCheckBox("Show Bounding Box");
    m_chkGrid->setChecked(true);
    m_chkAxis->setChecked(true);
    m_chkBBox->setChecked(true);
    modeLayout->addWidget(m_cmbRenderMode);
    modeLayout->addWidget(m_chkGrid);
    modeLayout->addWidget(m_chkAxis);
    modeLayout->addWidget(m_chkBBox);

    // 渲染参数组
    QGroupBox *grpParams = new QGroupBox("Render Params");
    QFormLayout *paramsLayout = new QFormLayout(grpParams);
    m_spnPointSize = new QDoubleSpinBox;
    m_spnPointSize->setRange(1.0, 20.0);
    m_spnPointSize->setValue(3.0);
    m_spnPointSize->setSingleStep(0.5);
    m_spnSplatScale = new QDoubleSpinBox;
    m_spnSplatScale->setRange(0.01, 10.0);
    m_spnSplatScale->setValue(1.0);
    m_spnSplatScale->setSingleStep(0.1);
    m_spnAlphaScale = new QDoubleSpinBox;
    m_spnAlphaScale->setRange(0.01, 5.0);
    m_spnAlphaScale->setValue(1.0);
    m_spnAlphaScale->setSingleStep(0.1);
    paramsLayout->addRow("Point Size:", m_spnPointSize);
    paramsLayout->addRow("Splat Scale:", m_spnSplatScale);
    paramsLayout->addRow("Alpha Scale:", m_spnAlphaScale);

    leftLayout->addWidget(grpFile);
    leftLayout->addWidget(grpView);
    leftLayout->addWidget(grpMode);
    leftLayout->addWidget(grpParams);
    leftLayout->addStretch();

    // ===== 中间 OpenGL 视口 =====
    m_renderWidget = new RenderWidget;
    m_renderWidget->setMinimumSize(600, 500);

    // ===== 右侧统计面板 =====
    QWidget *rightPanel = new QWidget;
    rightPanel->setFixedWidth(280);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(4, 4, 4, 4);
    rightLayout->setSpacing(6);

    auto makeLabel = [](const QString &text) -> QLabel* {
        QLabel *l = new QLabel(text);
        l->setObjectName("value");
        l->setWordWrap(true);
        return l;
    };

    // Gaussian 信息组
    QGroupBox *grpInfo = new QGroupBox("Gaussian Info");
    QFormLayout *infoLayout = new QFormLayout(grpInfo);
    m_lblFilePath = makeLabel("—");
    m_lblFormat = makeLabel("—");
    m_lblCount = makeLabel("—");
    m_lblHasOpacity = makeLabel("—");
    m_lblHasScale = makeLabel("—");
    m_lblHasRotation = makeLabel("—");
    m_lblHasSH = makeLabel("—");
    m_lblLoadTime = makeLabel("—");
    infoLayout->addRow("File:", m_lblFilePath);
    infoLayout->addRow("Format:", m_lblFormat);
    infoLayout->addRow("Count:", m_lblCount);
    infoLayout->addRow("Opacity:", m_lblHasOpacity);
    infoLayout->addRow("Scale:", m_lblHasScale);
    infoLayout->addRow("Rotation:", m_lblHasRotation);
    infoLayout->addRow("SH/f_dc:", m_lblHasSH);
    infoLayout->addRow("Load Time:", m_lblLoadTime);

    // 包围盒组
    QGroupBox *grpBBox = new QGroupBox("Bounding Box");
    QFormLayout *bboxLayout = new QFormLayout(grpBBox);
    m_lblBBoxMin = makeLabel("—");
    m_lblBBoxMax = makeLabel("—");
    m_lblBBoxSize = makeLabel("—");
    bboxLayout->addRow("Min:", m_lblBBoxMin);
    bboxLayout->addRow("Max:", m_lblBBoxMax);
    bboxLayout->addRow("Size:", m_lblBBoxSize);

    // 性能组
    QGroupBox *grpPerf = new QGroupBox("Performance");
    QFormLayout *perfLayout = new QFormLayout(grpPerf);
    m_lblFps = makeLabel("—");
    m_lblFrameMs = makeLabel("—");
    m_lblRenderMode = makeLabel("—");
    m_lblPointSize = makeLabel("—");
    m_lblVram = makeLabel("—");
    perfLayout->addRow("FPS:", m_lblFps);
    perfLayout->addRow("Frame ms:", m_lblFrameMs);
    perfLayout->addRow("Mode:", m_lblRenderMode);
    perfLayout->addRow("Point/Splat:", m_lblPointSize);
    perfLayout->addRow("VRAM est.:", m_lblVram);

    // 关系说明组
    QGroupBox *grpRelation = new QGroupBox("Mesh-GS Relation");
    QVBoxLayout *relLayout = new QVBoxLayout(grpRelation);
    m_lblRelation = new QLabel("Demo-05 loads Gaussian assets only. No photogrammetry mesh is loaded. Mesh+GS fusion is reserved for Demo-06.");
    m_lblRelation->setWordWrap(true);
    m_lblRelation->setStyleSheet("color: #80C0FF; font-size: 11px;");
    m_lblFutureAlign = makeLabel("prepared_for_demo06_alignment");
    m_lblFutureAlign->setStyleSheet("color: #FFD080; font-size: 11px;");
    relLayout->addWidget(m_lblRelation);
    relLayout->addWidget(new QLabel("Future Alignment:"));
    relLayout->addWidget(m_lblFutureAlign);

    rightLayout->addWidget(grpInfo);
    rightLayout->addWidget(grpBBox);
    rightLayout->addWidget(grpPerf);
    rightLayout->addWidget(grpRelation);
    rightLayout->addStretch();

    // ===== 底部日志 =====
    m_logEdit = new QTextEdit;
    m_logEdit->setReadOnly(true);
    m_logEdit->setFixedHeight(100);

    // ===== 组装布局 =====
    QSplitter *hSplitter = new QSplitter(Qt::Horizontal);
    hSplitter->addWidget(leftPanel);
    hSplitter->addWidget(m_renderWidget);
    hSplitter->addWidget(rightPanel);
    hSplitter->setStretchFactor(1, 1);

    QVBoxLayout *outerLayout = new QVBoxLayout;
    outerLayout->addWidget(hSplitter, 1);
    outerLayout->addWidget(m_logEdit);
    mainLayout->addLayout(outerLayout);

    // ===== 信号连接 =====
    connect(m_btnOpen, &QPushButton::clicked, this, &MainWindow::onOpenGaussian);
    connect(m_btnReload, &QPushButton::clicked, this, &MainWindow::onReload);
    connect(m_btnResetView, &QPushButton::clicked, this, &MainWindow::onResetView);
    connect(m_btnFitToCloud, &QPushButton::clicked, this, &MainWindow::onFitToCloud);
    connect(m_btnScreenshot, &QPushButton::clicked, this, &MainWindow::onTakeScreenshot);
    connect(m_cmbRenderMode, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onRenderModeChanged);
    connect(m_chkGrid, &QCheckBox::toggled, this, &MainWindow::onShowGridToggled);
    connect(m_chkAxis, &QCheckBox::toggled, this, &MainWindow::onShowAxisToggled);
    connect(m_chkBBox, &QCheckBox::toggled, this, &MainWindow::onShowBBoxToggled);
    connect(m_spnPointSize, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onPointSizeChanged);
    connect(m_spnSplatScale, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onSplatScaleChanged);
    connect(m_spnAlphaScale, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onAlphaScaleChanged);
    connect(m_renderWidget, &RenderWidget::fpsUpdated, this, &MainWindow::onFpsUpdated);

    appendLog("[Demo-05] Gaussian Splat Viewer initialized. Open a PLY file to begin.");
}

void MainWindow::onOpenGaussian() {
    QString path = QFileDialog::getOpenFileName(this, "Open Gaussian File",
        QString(), "Gaussian Files (*.ply *.splat);;All Files (*)");
    if (!path.isEmpty()) loadGaussian(path);
}

void MainWindow::onReload() {
    if (!m_currentFile.isEmpty()) loadGaussian(m_currentFile);
}

void MainWindow::onResetView() { m_renderWidget->resetView(); }
void MainWindow::onFitToCloud() { m_renderWidget->fitToCloud(); }

void MainWindow::onRenderModeChanged(int idx) {
    RenderMode mode = (idx == 0) ? RenderMode::Splat :
                      (idx == 1) ? RenderMode::Point : RenderMode::DebugColor;
    m_renderWidget->setRenderMode(mode);
    updateStatsPanel();
}

void MainWindow::onShowGridToggled(bool v) { m_renderWidget->setShowGrid(v); }
void MainWindow::onShowAxisToggled(bool v) { m_renderWidget->setShowAxis(v); }
void MainWindow::onShowBBoxToggled(bool v) { m_renderWidget->setShowBBox(v); }
void MainWindow::onPointSizeChanged(double v) { m_renderWidget->setPointSize((float)v); updateStatsPanel(); }
void MainWindow::onSplatScaleChanged(double v) { m_renderWidget->setSplatScale((float)v); updateStatsPanel(); }
void MainWindow::onAlphaScaleChanged(double v) { m_renderWidget->setAlphaScale((float)v); }

void MainWindow::onTakeScreenshot() {
    QString ts = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString path = QString("screenshot_%1.png").arg(ts);
    m_renderWidget->saveScreenshot(path);
    appendLog(QString("[Screenshot] Saved: %1").arg(path));
}

void MainWindow::onFpsUpdated(float fps, float frameMs) {
    m_lblFps->setText(QString("%1 fps").arg(fps, 0, 'f', 1));
    m_lblFrameMs->setText(QString("%1 ms").arg(frameMs, 0, 'f', 2));
}

void MainWindow::loadGaussian(const QString &path) {
    appendLog(QString("[Load] Loading: %1").arg(path));

    auto cloud = std::make_shared<GaussianCloud>();
    QString errMsg;
    bool ok = false;

    QFileInfo fi(path);
    QString ext = fi.suffix().toLower();

    if (ext == "ply") {
        ok = GaussianPlyLoader::load(path, *cloud, errMsg);
    } else if (ext == "splat") {
        ok = GaussianSplatLoader::load(path, *cloud, errMsg);
    } else {
        // 尝试 PLY
        ok = GaussianPlyLoader::load(path, *cloud, errMsg);
        if (!ok) ok = GaussianSplatLoader::load(path, *cloud, errMsg);
    }

    if (!ok) {
        appendLog(QString("[Error] Failed to load: %1").arg(errMsg));
        QMessageBox::warning(this, "Load Failed", errMsg);
        return;
    }

    m_cloud = cloud;
    m_currentFile = path;
    m_btnReload->setEnabled(true);

    m_renderWidget->loadCloud(m_cloud);
    m_renderWidget->fitToCloud();
    updateStatsPanel();

    appendLog(QString("[Load] OK: %1 Gaussians, format=%2, time=%3ms")
        .arg(cloud->count()).arg(cloud->formatType).arg(cloud->loadTimeMs));

    if (!cloud->hasOpacity)
        appendLog("[Warning] No opacity field found, using default opacity=1.0");
    if (!cloud->hasScale)
        appendLog("[Warning] No scale field found, using default scale=0.05");
    if (!cloud->hasRotation)
        appendLog("[Warning] No rotation field found, using identity quaternion");
    if (!cloud->hasSH)
        appendLog("[Warning] No SH/f_dc field found, using RGB color directly");
}

void MainWindow::updateStatsPanel() {
    if (!m_cloud) return;

    QFileInfo fi(m_currentFile);
    m_lblFilePath->setText(fi.fileName());
    m_lblFormat->setText(m_cloud->formatType);
    m_lblCount->setText(QString::number(m_cloud->count()));
    m_lblHasOpacity->setText(m_cloud->hasOpacity ? "Yes" : "No (default 1.0)");
    m_lblHasScale->setText(m_cloud->hasScale ? "Yes" : "No (default 0.05)");
    m_lblHasRotation->setText(m_cloud->hasRotation ? "Yes" : "No (identity)");
    m_lblHasSH->setText(m_cloud->hasSH ? "Yes" : "No (RGB direct)");
    m_lblLoadTime->setText(QString("%1 ms").arg(m_cloud->loadTimeMs));

    auto fmt3 = [](const QVector3D &v) {
        return QString("(%1, %2, %3)")
            .arg(v.x(), 0, 'f', 3)
            .arg(v.y(), 0, 'f', 3)
            .arg(v.z(), 0, 'f', 3);
    };
    m_lblBBoxMin->setText(fmt3(m_cloud->bboxMin()));
    m_lblBBoxMax->setText(fmt3(m_cloud->bboxMax()));
    m_lblBBoxSize->setText(fmt3(m_cloud->bboxSize()));

    RenderMode mode = m_renderWidget->renderMode();
    QString modeStr = (mode == RenderMode::Splat) ? "Splat/Billboard" :
                      (mode == RenderMode::Point) ? "Point" : "Debug Color";
    m_lblRenderMode->setText(modeStr);
    m_lblPointSize->setText(QString("pt=%.1f / splat=%.2f")
        .arg(m_spnPointSize->value()).arg(m_spnSplatScale->value()));
    m_lblVram->setText(estimateVramMB());
}

QString MainWindow::estimateVramMB() const {
    if (!m_cloud) return "—";
    // 每个 Gaussian 在 GPU 上约 28 bytes（point）+ 40 bytes（billboard instance）
    long long bytes = (long long)m_cloud->count() * (28 + 40);
    double mb = bytes / (1024.0 * 1024.0);
    return QString("%1 MB (est.)").arg(mb, 0, 'f', 1);
}

void MainWindow::appendLog(const QString &msg) {
    QString ts = QDateTime::currentDateTime().toString("hh:mm:ss");
    m_logEdit->append(QString("[%1] %2").arg(ts, msg));
}

void MainWindow::loadFromCommandLine(const QString &path) {
    if (!path.isEmpty()) {
        QTimer::singleShot(200, this, [this, path]() { loadGaussian(path); });
    }
}
