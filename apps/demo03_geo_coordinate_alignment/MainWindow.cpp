#include "MainWindow.h"
#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QSplitter>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QPixmap>
#include <QStatusBar>
#include <QTextBlock>

static QLabel* makeValueLabel(const QString& init = "--") {
    QLabel* lbl = new QLabel(init);
    lbl->setStyleSheet("color: #00E5FF; font-family: monospace; font-size: 11px;");
    lbl->setMinimumWidth(120);
    return lbl;
}

static QLabel* makeTitleLabel(const QString& text) {
    QLabel* lbl = new QLabel(text);
    lbl->setStyleSheet("color: #B0BEC5; font-size: 11px;");
    return lbl;
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setWindowTitle("Demo-03: WGS84/ENU/SCENE Geo Coordinate Alignment");
    resize(1400, 800);

    m_receiver = new UdpWgs84Receiver(this);
    connect(m_receiver, &UdpWgs84Receiver::stateReceived, this, &MainWindow::onStateReceived);
    connect(m_receiver, &UdpWgs84Receiver::logMessage,    this, &MainWindow::onUdpLog);
    connect(m_receiver, &UdpWgs84Receiver::errorOccurred, this, &MainWindow::onUdpError);

    setupUI();
    applyDarkTheme();

    // 默认原点
    m_editOriginLat->setText("30.700000");
    m_editOriginLon->setText("104.000000");
    m_editOriginAlt->setText("500.0");
    m_editUdpPort->setText("14581");
    m_editSceneScale->setText("1.0");
    applyOriginFromUI();

    appendLog("[Demo-03] WGS84/ENU/SCENE Geo Coordinate Alignment started");
    appendLog("[Demo-03] Default origin: lat=30.700000, lon=104.000000, alt=500.0m");
    appendLog("[Demo-03] Click 'Start UDP' then run: python3 tools/geo_telemetry_simulator/send_wgs84_uav_udp.py");
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI() {
    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    QHBoxLayout* mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);

    // ===== 左侧配置面板 =====
    QWidget* leftPanel = new QWidget();
    leftPanel->setFixedWidth(220);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(4, 4, 4, 4);
    leftLayout->setSpacing(6);

    // 地理原点配置
    QGroupBox* gbOrigin = new QGroupBox("地理参考原点 (Geo Origin)");
    QFormLayout* originForm = new QFormLayout(gbOrigin);
    originForm->setSpacing(4);
    m_editOriginLat  = new QLineEdit(); m_editOriginLat->setPlaceholderText("30.700000");
    m_editOriginLon  = new QLineEdit(); m_editOriginLon->setPlaceholderText("104.000000");
    m_editOriginAlt  = new QLineEdit(); m_editOriginAlt->setPlaceholderText("500.0");
    m_editUdpPort    = new QLineEdit(); m_editUdpPort->setPlaceholderText("14581");
    m_editSceneScale = new QLineEdit(); m_editSceneScale->setPlaceholderText("1.0");
    originForm->addRow("Origin Lat:",   m_editOriginLat);
    originForm->addRow("Origin Lon:",   m_editOriginLon);
    originForm->addRow("Origin Alt(m):", m_editOriginAlt);
    originForm->addRow("UDP Port:",     m_editUdpPort);
    originForm->addRow("Scene Scale:",  m_editSceneScale);
    leftLayout->addWidget(gbOrigin);

    // 显示选项
    QGroupBox* gbDisplay = new QGroupBox("显示选项 (Display)");
    QVBoxLayout* dispLayout = new QVBoxLayout(gbDisplay);
    m_cbGrid        = new QCheckBox("地面网格 (Grid)");       m_cbGrid->setChecked(true);
    m_cbAxis        = new QCheckBox("ENU 坐标轴 (Axis)");     m_cbAxis->setChecked(true);
    m_cbOriginMarker= new QCheckBox("原点标记 (Origin)");     m_cbOriginMarker->setChecked(true);
    m_cbRings       = new QCheckBox("距离环 (Distance Rings)");m_cbRings->setChecked(true);
    connect(m_cbGrid,         &QCheckBox::toggled, this, [this](bool v){ m_renderWidget->setShowGrid(v); });
    connect(m_cbAxis,         &QCheckBox::toggled, this, [this](bool v){ m_renderWidget->setShowAxis(v); });
    connect(m_cbOriginMarker, &QCheckBox::toggled, this, [this](bool v){ m_renderWidget->setShowOriginMarker(v); });
    connect(m_cbRings,        &QCheckBox::toggled, this, [this](bool v){ m_renderWidget->setShowDistanceRings(v); });
    dispLayout->addWidget(m_cbGrid);
    dispLayout->addWidget(m_cbAxis);
    dispLayout->addWidget(m_cbOriginMarker);
    dispLayout->addWidget(m_cbRings);
    leftLayout->addWidget(gbDisplay);

    // 操作按钮
    QGroupBox* gbCtrl = new QGroupBox("操作 (Controls)");
    QVBoxLayout* ctrlLayout = new QVBoxLayout(gbCtrl);
    ctrlLayout->setSpacing(4);
    m_btnLoadConfig  = new QPushButton("Load Geo Config");
    m_btnResetOrigin = new QPushButton("Apply Origin");
    m_btnStartUdp    = new QPushButton("Start UDP");
    m_btnStopUdp     = new QPushButton("Stop UDP");
    m_btnClearTraj   = new QPushButton("Clear Trajectory");
    m_btnResetView   = new QPushButton("Reset View");
    m_btnFitTraj     = new QPushButton("Fit To Trajectory");
    m_btnScreenshot  = new QPushButton("Take Screenshot");
    m_btnValidate    = new QPushButton("Validate Origin");
    m_btnStopUdp->setEnabled(false);
    connect(m_btnLoadConfig,  &QPushButton::clicked, this, &MainWindow::onLoadConfig);
    connect(m_btnResetOrigin, &QPushButton::clicked, this, &MainWindow::onResetOrigin);
    connect(m_btnStartUdp,    &QPushButton::clicked, this, &MainWindow::onStartUdp);
    connect(m_btnStopUdp,     &QPushButton::clicked, this, &MainWindow::onStopUdp);
    connect(m_btnClearTraj,   &QPushButton::clicked, this, &MainWindow::onClearTrajectory);
    connect(m_btnResetView,   &QPushButton::clicked, this, &MainWindow::onResetView);
    connect(m_btnFitTraj,     &QPushButton::clicked, this, &MainWindow::onFitToTrajectory);
    connect(m_btnScreenshot,  &QPushButton::clicked, this, &MainWindow::onTakeScreenshot);
    connect(m_btnValidate,    &QPushButton::clicked, this, &MainWindow::onValidateOrigin);
    for (auto* btn : {m_btnLoadConfig, m_btnResetOrigin, m_btnStartUdp, m_btnStopUdp,
                      m_btnClearTraj, m_btnResetView, m_btnFitTraj, m_btnScreenshot, m_btnValidate}) {
        btn->setFixedHeight(28);
        ctrlLayout->addWidget(btn);
    }
    leftLayout->addWidget(gbCtrl);
    leftLayout->addStretch();

    // UDP 状态
    m_lblUdpStatus = new QLabel("UDP: Stopped");
    m_lblUdpStatus->setStyleSheet("color: #EF5350; font-size: 11px; font-weight: bold;");
    leftLayout->addWidget(m_lblUdpStatus);

    // ===== 中间三维视口 =====
    m_renderWidget = new RenderWidget();

    // ===== 右侧坐标结果面板 =====
    QWidget* rightPanel = new QWidget();
    rightPanel->setFixedWidth(230);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(4, 4, 4, 4);
    rightLayout->setSpacing(4);

    auto addCoordRow = [&](QFormLayout* form, const QString& label, QLabel*& lbl) {
        lbl = makeValueLabel();
        form->addRow(makeTitleLabel(label), lbl);
    };

    // UAV 信息
    QGroupBox* gbUav = new QGroupBox("无人机信息 (UAV Info)");
    QFormLayout* uavForm = new QFormLayout(gbUav);
    uavForm->setSpacing(3);
    addCoordRow(uavForm, "UAV ID:",      m_lblUavId);
    addCoordRow(uavForm, "Timestamp:",   m_lblTimestamp);
    addCoordRow(uavForm, "Yaw (°):",     m_lblYaw);
    addCoordRow(uavForm, "Battery (%):", m_lblBattery);
    rightLayout->addWidget(gbUav);

    // WGS84 原始坐标
    QGroupBox* gbWgs = new QGroupBox("WGS84 原始坐标");
    QFormLayout* wgsForm = new QFormLayout(gbWgs);
    wgsForm->setSpacing(3);
    addCoordRow(wgsForm, "Latitude (°):",  m_lblRawLat);
    addCoordRow(wgsForm, "Longitude (°):", m_lblRawLon);
    addCoordRow(wgsForm, "Altitude (m):",  m_lblRawAlt);
    rightLayout->addWidget(gbWgs);

    // ENU 坐标
    QGroupBox* gbEnu = new QGroupBox("ENU 局部坐标 (m)");
    QFormLayout* enuForm = new QFormLayout(gbEnu);
    enuForm->setSpacing(3);
    addCoordRow(enuForm, "East (m):",  m_lblEnuE);
    addCoordRow(enuForm, "North (m):", m_lblEnuN);
    addCoordRow(enuForm, "Up (m):",    m_lblEnuU);
    rightLayout->addWidget(gbEnu);

    // SCENE 坐标
    QGroupBox* gbScene = new QGroupBox("SCENE 坐标 (m)");
    QFormLayout* sceneForm = new QFormLayout(gbScene);
    sceneForm->setSpacing(3);
    addCoordRow(sceneForm, "SCENE X:", m_lblSceneX);
    addCoordRow(sceneForm, "SCENE Y:", m_lblSceneY);
    addCoordRow(sceneForm, "SCENE Z:", m_lblSceneZ);
    rightLayout->addWidget(gbScene);

    // OpenGL 坐标
    QGroupBox* gbGl = new QGroupBox("OpenGL 显示坐标");
    QFormLayout* glForm = new QFormLayout(gbGl);
    glForm->setSpacing(3);
    addCoordRow(glForm, "GL X:", m_lblGlX);
    addCoordRow(glForm, "GL Y:", m_lblGlY);
    addCoordRow(glForm, "GL Z:", m_lblGlZ);
    rightLayout->addWidget(gbGl);

    // 距离与方位
    QGroupBox* gbDist = new QGroupBox("距离与方位 (Distance)");
    QFormLayout* distForm = new QFormLayout(gbDist);
    distForm->setSpacing(3);
    addCoordRow(distForm, "Horizontal (m):", m_lblDistH);
    addCoordRow(distForm, "Total 3D (m):",   m_lblDistTotal);
    addCoordRow(distForm, "Azimuth (°):",    m_lblAzimuth);
    rightLayout->addWidget(gbDist);
    rightLayout->addStretch();

    // ===== 底部日志 =====
    m_logEdit = new QTextEdit();
    m_logEdit->setReadOnly(true);
    m_logEdit->setFixedHeight(100);
    m_logEdit->setStyleSheet("background: #0D1117; color: #8B949E; font-family: monospace; font-size: 10px;");

    // ===== 整体布局 =====
    QSplitter* hSplitter = new QSplitter(Qt::Horizontal);
    hSplitter->addWidget(leftPanel);
    hSplitter->addWidget(m_renderWidget);
    hSplitter->addWidget(rightPanel);
    hSplitter->setStretchFactor(0, 0);
    hSplitter->setStretchFactor(1, 1);
    hSplitter->setStretchFactor(2, 0);

    QVBoxLayout* vLayout = new QVBoxLayout();
    vLayout->addWidget(hSplitter, 1);
    vLayout->addWidget(m_logEdit, 0);
    mainLayout->addLayout(vLayout);

    statusBar()->showMessage("Demo-03: WGS84/ENU/SCENE Geo Coordinate Alignment  |  Port: 14581");
}

void MainWindow::applyDarkTheme() {
    qApp->setStyle("Fusion");
    QString qss = R"(
        QWidget { background: #1E1E2E; color: #E0E0E0; font-size: 12px; }
        QGroupBox { border: 1px solid #3A3A5C; border-radius: 4px; margin-top: 8px;
                    padding-top: 6px; font-size: 11px; color: #90CAF9; }
        QGroupBox::title { subcontrol-origin: margin; left: 8px; padding: 0 4px; }
        QLineEdit { background: #2D2D44; border: 1px solid #4A4A6A; border-radius: 3px;
                    padding: 2px 4px; color: #E0E0E0; }
        QPushButton { background: #2D3748; border: 1px solid #4A5568; border-radius: 3px;
                      padding: 4px 8px; color: #CBD5E0; }
        QPushButton:hover { background: #3D4A5C; }
        QPushButton:pressed { background: #1A2535; }
        QPushButton:disabled { color: #555; background: #222; }
        QCheckBox { color: #B0BEC5; }
        QTextEdit { background: #0D1117; border: 1px solid #30363D; color: #8B949E; }
        QSplitter::handle { background: #2D2D44; }
        QLabel { color: #E0E0E0; }
        QFormLayout { spacing: 3px; }
    )";
    setStyleSheet(qss);
}

void MainWindow::onLoadConfig() {
    QString path = QFileDialog::getOpenFileName(this, "Load Geo Config", ".", "JSON (*.json)");
    if (!path.isEmpty()) loadConfigFile(path);
}

void MainWindow::loadConfigFile(const QString& path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        appendLog(QString("[Config] Cannot open: %1").arg(path));
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isObject()) {
        appendLog("[Config] Invalid JSON format");
        return;
    }
    QJsonObject root = doc.object();
    if (root.contains("geo_origin")) {
        QJsonObject geo = root["geo_origin"].toObject();
        m_editOriginLat->setText(QString::number(geo["lat"].toDouble(), 'f', 8));
        m_editOriginLon->setText(QString::number(geo["lon"].toDouble(), 'f', 8));
        m_editOriginAlt->setText(QString::number(geo["alt"].toDouble(), 'f', 1));
    }
    if (root.contains("telemetry")) {
        QJsonObject tel = root["telemetry"].toObject();
        m_editUdpPort->setText(QString::number(tel["port"].toInt(14581)));
    }
    if (root.contains("scene")) {
        QJsonObject sc = root["scene"].toObject();
        m_editSceneScale->setText(QString::number(sc["scene_scale"].toDouble(1.0), 'f', 2));
    }
    applyOriginFromUI();
    appendLog(QString("[Config] Loaded: %1").arg(path));
}

void MainWindow::applyOriginFromUI() {
    Wgs84Coord origin;
    origin.lat = m_editOriginLat->text().toDouble();
    origin.lon = m_editOriginLon->text().toDouble();
    origin.alt = m_editOriginAlt->text().toDouble();
    double scale = m_editSceneScale->text().toDouble();
    if (scale <= 0) scale = 1.0;
    m_transformer.setOrigin(origin);
    m_transformer.setSceneScale(scale);
    appendLog(QString("[Origin] Set to lat=%1 lon=%2 alt=%3m scale=%4")
              .arg(origin.lat, 0, 'f', 6).arg(origin.lon, 0, 'f', 6)
              .arg(origin.alt, 0, 'f', 1).arg(scale, 0, 'f', 2));
}

void MainWindow::onResetOrigin() {
    applyOriginFromUI();
}

void MainWindow::onStartUdp() {
    quint16 port = static_cast<quint16>(m_editUdpPort->text().toUInt());
    if (port == 0) port = 14581;
    if (m_receiver->startListening(port)) {
        m_btnStartUdp->setEnabled(false);
        m_btnStopUdp->setEnabled(true);
        m_lblUdpStatus->setText(QString("UDP: Listening :%1").arg(port));
        m_lblUdpStatus->setStyleSheet("color: #66BB6A; font-size: 11px; font-weight: bold;");
    }
}

void MainWindow::onStopUdp() {
    m_receiver->stopListening();
    m_btnStartUdp->setEnabled(true);
    m_btnStopUdp->setEnabled(false);
    m_lblUdpStatus->setText("UDP: Stopped");
    m_lblUdpStatus->setStyleSheet("color: #EF5350; font-size: 11px; font-weight: bold;");
}

void MainWindow::onClearTrajectory() {
    m_renderWidget->clearTrajectory();
    m_packetCount = 0;
    appendLog("[Trajectory] Cleared");
}

void MainWindow::onResetView() {
    m_renderWidget->resetView();
}

void MainWindow::onFitToTrajectory() {
    m_renderWidget->fitToTrajectory();
}

void MainWindow::onTakeScreenshot() {
    QString dir = QDir::currentPath();
    QString filename = QString("demo03_screenshot_%1.png")
                       .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    QString path = QDir(dir).filePath(filename);
    QImage img = m_renderWidget->takeScreenshot();
    if (img.save(path)) {
        appendLog(QString("[Screenshot] Saved: %1").arg(path));
        QMessageBox::information(this, "Screenshot", QString("Saved to:\n%1").arg(path));
    } else {
        appendLog(QString("[Screenshot] Failed to save: %1").arg(path));
    }
}

void MainWindow::onValidateOrigin() {
    if (!m_transformer.hasOrigin()) {
        appendLog("[Validate] No origin set");
        return;
    }
    TransformResult r = m_transformer.validateOrigin();
    if (r.valid) {
        appendLog(QString("[Validate] Origin -> ENU: E=%1 N=%2 U=%3 (expected ~0,0,0)")
                  .arg(r.enu.e, 0, 'f', 4).arg(r.enu.n, 0, 'f', 4).arg(r.enu.u, 0, 'f', 4));
        appendLog(QString("[Validate] Distance to origin: %1m (expected ~0m)").arg(r.distanceTotal, 0, 'f', 4));
        // 小偏移测试点：向东约 10m，向北约 20m，高度 +5m
        Wgs84Coord testPt;
        testPt.lat = m_transformer.getOrigin().lat + 20.0 / 6378137.0 * (180.0 / M_PI);
        testPt.lon = m_transformer.getOrigin().lon + 10.0 / (6378137.0 * std::cos(m_transformer.getOrigin().lat * M_PI / 180.0)) * (180.0 / M_PI);
        testPt.alt = m_transformer.getOrigin().alt + 5.0;
        TransformResult r2 = m_transformer.transform(testPt);
        if (r2.valid) {
            appendLog(QString("[Validate] Test point (+10E +20N +5U) -> ENU: E=%1 N=%2 U=%3 (expected ~10,20,5)")
                      .arg(r2.enu.e, 0, 'f', 2).arg(r2.enu.n, 0, 'f', 2).arg(r2.enu.u, 0, 'f', 2));
        }
    } else {
        appendLog(QString("[Validate] Error: %1").arg(r.errorMsg));
    }
}

void MainWindow::onStateReceived(const GeoUavState& state) {
    if (!state.valid) return;

    Wgs84Coord wgs;
    wgs.lat = state.lat;
    wgs.lon = state.lon;
    wgs.alt = state.alt;

    TransformResult result = m_transformer.transform(wgs);
    if (!result.valid) {
        appendLog(QString("[Error] Transform failed: %1").arg(result.errorMsg));
        return;
    }

    // 更新三维视口
    m_renderWidget->updateUavState(result.opengl, static_cast<float>(state.yaw));

    // 更新坐标面板
    updateCoordPanel(result);

    // 更新 UAV 信息
    m_lblUavId->setText(state.uavId);
    m_lblTimestamp->setText(QString::number(state.timestampMs));
    m_lblYaw->setText(QString::number(state.yaw, 'f', 1) + "°");
    m_lblBattery->setText(QString::number(state.battery, 'f', 1) + "%");

    m_packetCount++;
    if (m_packetCount % 50 == 0) {
        appendLog(QString("[UDP] Received %1 packets | Last ENU: E=%2 N=%3 U=%4")
                  .arg(m_packetCount)
                  .arg(result.enu.e, 0, 'f', 1)
                  .arg(result.enu.n, 0, 'f', 1)
                  .arg(result.enu.u, 0, 'f', 1));
    }
}

void MainWindow::updateCoordPanel(const TransformResult& result) {
    m_lblRawLat->setText(QString::number(result.wgs84.lat, 'f', 8));
    m_lblRawLon->setText(QString::number(result.wgs84.lon, 'f', 8));
    m_lblRawAlt->setText(QString::number(result.wgs84.alt, 'f', 2) + " m");

    m_lblEnuE->setText(QString::number(result.enu.e, 'f', 3) + " m");
    m_lblEnuN->setText(QString::number(result.enu.n, 'f', 3) + " m");
    m_lblEnuU->setText(QString::number(result.enu.u, 'f', 3) + " m");

    m_lblSceneX->setText(QString::number(static_cast<double>(result.scene.x), 'f', 3) + " m");
    m_lblSceneY->setText(QString::number(static_cast<double>(result.scene.y), 'f', 3) + " m");
    m_lblSceneZ->setText(QString::number(static_cast<double>(result.scene.z), 'f', 3) + " m");

    m_lblGlX->setText(QString::number(static_cast<double>(result.opengl.x), 'f', 3));
    m_lblGlY->setText(QString::number(static_cast<double>(result.opengl.y), 'f', 3));
    m_lblGlZ->setText(QString::number(static_cast<double>(result.opengl.z), 'f', 3));

    m_lblDistH->setText(QString::number(result.distanceHorizontal, 'f', 2) + " m");
    m_lblDistTotal->setText(QString::number(result.distanceTotal, 'f', 2) + " m");
    m_lblAzimuth->setText(QString::number(result.azimuth_deg, 'f', 1) + "°");
}

void MainWindow::onUdpLog(const QString& msg) {
    appendLog(msg);
}

void MainWindow::onUdpError(const QString& msg) {
    appendLog(msg);
    m_lblUdpStatus->setText("UDP: Error");
    m_lblUdpStatus->setStyleSheet("color: #EF5350; font-size: 11px; font-weight: bold;");
}

void MainWindow::appendLog(const QString& msg) {
    QString ts = QDateTime::currentDateTime().toString("hh:mm:ss");
    m_logEdit->append(QString("[%1] %2").arg(ts).arg(msg));
    // 保持最多 200 行
    QTextDocument* doc = m_logEdit->document();
    while (doc->blockCount() > 200) {
        QTextCursor cursor(doc->begin());
        cursor.select(QTextCursor::BlockUnderCursor);
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
    }
}
