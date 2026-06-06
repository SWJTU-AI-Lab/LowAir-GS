#include "MainWindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QSplitter>
#include <QGroupBox>
#include <QApplication>
#include <QDateTime>
#include <QFileInfo>
#include <QDir>
#include <cmath>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Demo-04 MAVLink Telemetry Viewer — LowAir-GS");
    resize(1400, 860);
    applyDarkTheme();

    // 先构建 UI（所有指针在此初始化）
    buildUi();

    // 遥测源
    m_source   = new MavlinkRawTelemetrySource(this);
    m_recorder = new TelemetryRecorder(this);

    connect(m_source, &MavlinkRawTelemetrySource::telemetryUpdated,
            this,     &MainWindow::onTelemetryUpdated);
    connect(m_source, &MavlinkRawTelemetrySource::connectionStatusChanged,
            this,     &MainWindow::onConnectionStatusChanged);
    connect(m_source, &MavlinkRawTelemetrySource::errorOccurred,
            this,     &MainWindow::onSourceError);
    connect(m_source, &MavlinkRawTelemetrySource::logMessage,
            this,     &MainWindow::onLogMessage);
    connect(m_recorder, &TelemetryRecorder::logSaved,
            this,       &MainWindow::onLogSaved);

    // 频率统计定时器（UI 已初始化后再启动）
    m_freqTimer = new QTimer(this);
    m_freqTimer->setInterval(1000);
    connect(m_freqTimer, &QTimer::timeout, this, &MainWindow::onFreqTimer);
    m_freqTimer->start();

    // 设置默认 geo 原点
    Wgs84Coord origin;
    origin.lat = 30.700000;
    origin.lon = 104.000000;
    origin.alt = 500.0;
    m_transformer.setOrigin(origin);

    appendLog("[Demo-04] MAVLink Telemetry Viewer started (read-only, no control commands)");
    appendLog("[Demo-04] Default origin: 30.700000°N, 104.000000°E, alt=500m");
    appendLog("[Demo-04] Safety boundary: no flight control commands are implemented or sent.");
}

MainWindow::~MainWindow() {}

void MainWindow::buildUi() {
    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    QHBoxLayout* mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(6, 6, 6, 6);
    mainLayout->setSpacing(0);

    // ── 左侧控制面板 ──────────────────────────────────────────────
    QWidget* leftPanel = new QWidget(central);
    leftPanel->setFixedWidth(345);
    leftPanel->setStyleSheet("background: #1c1c20;");
    QVBoxLayout* lv = new QVBoxLayout(leftPanel);
    lv->setContentsMargins(6, 6, 6, 6);
    lv->setSpacing(5);

    // --- 连接配置 ---
    QGroupBox* connBox = new QGroupBox("连接配置 (MAVLink UDP)", leftPanel);
    QFormLayout* connForm = new QFormLayout(connBox);
    connForm->setSpacing(4);
    m_urlEdit = new QLineEdit("udp://:14550", connBox);
    connForm->addRow("Connection URL:", m_urlEdit);
    m_connectBtn    = new QPushButton("Connect",    connBox);
    m_disconnectBtn = new QPushButton("Disconnect", connBox);
    m_disconnectBtn->setEnabled(false);
    QHBoxLayout* btnRow = new QHBoxLayout();
    btnRow->addWidget(m_connectBtn);
    btnRow->addWidget(m_disconnectBtn);
    connForm->addRow(btnRow);
    m_statusLabel = new QLabel("● 未连接", connBox);
    m_statusLabel->setStyleSheet("color: #e05050; font-weight: bold;");
    connForm->addRow("状态:", m_statusLabel);
    lv->addWidget(connBox);

    // --- 地理原点配置 ---
    QGroupBox* originBox = new QGroupBox("地理原点 (WGS84)", leftPanel);
    QFormLayout* originForm = new QFormLayout(originBox);
    originForm->setSpacing(4);
    m_originLat = new QDoubleSpinBox(originBox);
    m_originLat->setRange(-90.0, 90.0);
    m_originLat->setDecimals(6);
    m_originLat->setValue(30.700000);
    m_originLon = new QDoubleSpinBox(originBox);
    m_originLon->setRange(-180.0, 180.0);
    m_originLon->setDecimals(6);
    m_originLon->setValue(104.000000);
    m_originAlt = new QDoubleSpinBox(originBox);
    m_originAlt->setRange(-500.0, 10000.0);
    m_originAlt->setDecimals(1);
    m_originAlt->setValue(500.0);
    originForm->addRow("Lat (°):", m_originLat);
    originForm->addRow("Lon (°):", m_originLon);
    originForm->addRow("Alt (m):", m_originAlt);
    QPushButton* applyOriginBtn = new QPushButton("Apply Origin", originBox);
    originForm->addRow(applyOriginBtn);
    connect(applyOriginBtn, &QPushButton::clicked, this, [this]() {
        Wgs84Coord o;
        o.lat = m_originLat->value();
        o.lon = m_originLon->value();
        o.alt = m_originAlt->value();
        m_transformer.setOrigin(o);
        m_renderWidget->clearTrajectory();
        appendLog(QString("[Geo] Origin: %1°N, %2°E, alt=%3m")
                  .arg(o.lat, 0, 'f', 6).arg(o.lon, 0, 'f', 6).arg(o.alt, 0, 'f', 1));
    });
    lv->addWidget(originBox);

    // --- 遥测状态面板 ---
    QGroupBox* stateBox = new QGroupBox("遥测状态", leftPanel);
    QFormLayout* sf = new QFormLayout(stateBox);
    sf->setSpacing(2);
    auto mkL = [&](const QString& init) -> QLabel* {
        QLabel* l = new QLabel(init, stateBox);
        l->setStyleSheet("color: #cccccc; font-family: monospace; font-size: 12px;");
        return l;
    };
    m_lblConnected = mkL("--"); m_lblGps    = mkL("--"); m_lblArmed  = mkL("--");
    m_lblMode      = mkL("--"); m_lblLat    = mkL("--"); m_lblLon    = mkL("--");
    m_lblAlt       = mkL("--"); m_lblRelAlt = mkL("--"); m_lblEnuE   = mkL("--");
    m_lblEnuN      = mkL("--"); m_lblEnuU   = mkL("--"); m_lblRoll   = mkL("--");
    m_lblPitch     = mkL("--"); m_lblYaw    = mkL("--"); m_lblBattery= mkL("--");
    m_lblDist      = mkL("--"); m_lblFreq   = mkL("--"); m_lblLogPath= mkL("--");
    sf->addRow("Connected:",   m_lblConnected);
    sf->addRow("GPS Valid:",   m_lblGps);
    sf->addRow("Armed:",       m_lblArmed);
    sf->addRow("Flight Mode:", m_lblMode);
    sf->addRow("Latitude:",    m_lblLat);
    sf->addRow("Longitude:",   m_lblLon);
    sf->addRow("Abs Alt (m):", m_lblAlt);
    sf->addRow("Rel Alt (m):", m_lblRelAlt);
    sf->addRow("ENU East:",    m_lblEnuE);
    sf->addRow("ENU North:",   m_lblEnuN);
    sf->addRow("ENU Up:",      m_lblEnuU);
    sf->addRow("Roll (°):",    m_lblRoll);
    sf->addRow("Pitch (°):",   m_lblPitch);
    sf->addRow("Yaw (°):",     m_lblYaw);
    sf->addRow("Battery (%):", m_lblBattery);
    sf->addRow("Dist (m):",    m_lblDist);
    sf->addRow("Telem Hz:",    m_lblFreq);
    sf->addRow("Log:",         m_lblLogPath);
    lv->addWidget(stateBox);

    // --- 日志控制 ---
    QGroupBox* logCtrlBox = new QGroupBox("日志控制", leftPanel);
    QHBoxLayout* lcr = new QHBoxLayout(logCtrlBox);
    m_startLogBtn = new QPushButton("Start Log", logCtrlBox);
    m_stopLogBtn  = new QPushButton("Stop Log",  logCtrlBox);
    m_stopLogBtn->setEnabled(false);
    lcr->addWidget(m_startLogBtn);
    lcr->addWidget(m_stopLogBtn);
    lv->addWidget(logCtrlBox);

    // --- 视图控制 ---
    QGroupBox* viewBox = new QGroupBox("视图控制", leftPanel);
    QHBoxLayout* vr = new QHBoxLayout(viewBox);
    m_clearTrajBtn = new QPushButton("Clear Traj", viewBox);
    m_resetViewBtn = new QPushButton("Reset View", viewBox);
    vr->addWidget(m_clearTrajBtn);
    vr->addWidget(m_resetViewBtn);
    lv->addWidget(viewBox);

    // --- 运行日志 ---
    QGroupBox* logBox = new QGroupBox("运行日志", leftPanel);
    QVBoxLayout* ll = new QVBoxLayout(logBox);
    m_logEdit = new QTextEdit(logBox);
    m_logEdit->setReadOnly(true);
    m_logEdit->setMaximumHeight(140);
    m_logEdit->setStyleSheet(
        "background: #141418; color: #aaaaaa; font-family: monospace; font-size: 11px;");
    ll->addWidget(m_logEdit);
    lv->addWidget(logBox);
    lv->addStretch();

    // ── 右侧 OpenGL 视口 ──────────────────────────────────────────
    m_renderWidget = new RenderWidget(central);

    QSplitter* splitter = new QSplitter(Qt::Horizontal, central);
    splitter->addWidget(leftPanel);
    splitter->addWidget(m_renderWidget);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    mainLayout->addWidget(splitter);

    // ── 信号连接 ──────────────────────────────────────────────────
    connect(m_connectBtn,    &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    connect(m_disconnectBtn, &QPushButton::clicked, this, &MainWindow::onDisconnectClicked);
    connect(m_startLogBtn,   &QPushButton::clicked, this, &MainWindow::onStartLogClicked);
    connect(m_stopLogBtn,    &QPushButton::clicked, this, &MainWindow::onStopLogClicked);
    connect(m_clearTrajBtn,  &QPushButton::clicked, this, &MainWindow::onClearTrajectoryClicked);
    connect(m_resetViewBtn,  &QPushButton::clicked, this, &MainWindow::onResetViewClicked);
}

void MainWindow::onConnectClicked() {
    QString url = m_urlEdit->text().trimmed();
    if (url.isEmpty()) url = "udp://:14550";
    m_connectBtn->setEnabled(false);
    m_disconnectBtn->setEnabled(true);
    m_source->start(url);
    appendLog(QString("[Connect] Starting MAVLink source on %1").arg(url));
}

void MainWindow::onDisconnectClicked() {
    m_source->stop();
    m_connectBtn->setEnabled(true);
    m_disconnectBtn->setEnabled(false);
    m_statusLabel->setText("● 未连接");
    m_statusLabel->setStyleSheet("color: #e05050; font-weight: bold;");
}

void MainWindow::onClearTrajectoryClicked() {
    m_renderWidget->clearTrajectory();
    appendLog("[View] Trajectory cleared");
}

void MainWindow::onResetViewClicked() {
    m_renderWidget->resetView();
}

void MainWindow::onStartLogClicked() {
    QString outDir = "outputs/demo04_logs";
    if (m_recorder->startRecording(outDir, "mavlink_uav_01")) {
        m_startLogBtn->setEnabled(false);
        m_stopLogBtn->setEnabled(true);
        m_lblLogPath->setText(QFileInfo(m_recorder->currentLogPath()).fileName());
        appendLog(QString("[Log] Recording started: %1").arg(m_recorder->currentLogPath()));
    }
}

void MainWindow::onStopLogClicked() {
    m_recorder->stopRecording();
    m_startLogBtn->setEnabled(true);
    m_stopLogBtn->setEnabled(false);
    appendLog("[Log] Recording stopped");
}

void MainWindow::onTelemetryUpdated(const DroneTelemetryState& rawState) {
    m_frameCount++;

    DroneTelemetryState state = rawState;
    if (state.gpsValid && m_transformer.hasOrigin()) {
        Wgs84Coord wgs84;
        wgs84.lat = state.latitudeDeg;
        wgs84.lon = state.longitudeDeg;
        wgs84.alt = state.absoluteAltitudeM;

        TransformResult res = m_transformer.transform(wgs84);
        state.enuEastM  = res.enu.e;
        state.enuNorthM = res.enu.n;
        state.enuUpM    = res.enu.u;
        state.sceneX    = res.scene.x;
        state.sceneY    = res.scene.y;
        state.sceneZ    = res.scene.z;
        state.openglX   = res.opengl.x;
        state.openglY   = res.opengl.y;
        state.openglZ   = res.opengl.z;
        state.horizontalDistanceToOriginM = res.distanceHorizontal;
        state.distanceToOrigin3dM         = res.distanceTotal;
    }

    m_renderWidget->updateUavState(
        QVector3D(static_cast<float>(state.openglX),
                  static_cast<float>(state.openglY),
                  static_cast<float>(state.openglZ)),
        static_cast<float>(state.yawDeg),
        state.connected,
        state.gpsValid
    );

    updateCoordPanel(state);

    if (m_recorder->isRecording()) {
        m_recorder->record(state);
    }
}

void MainWindow::updateCoordPanel(const DroneTelemetryState& s) {
    m_lblConnected->setText(s.connected ? "<font color='#50e050'>YES</font>" : "<font color='#e05050'>NO</font>");
    m_lblGps->setText(s.gpsValid ? "<font color='#50e050'>VALID</font>" : "<font color='#e05050'>INVALID</font>");
    m_lblArmed->setText(s.armed ? "<font color='#e0a050'>ARMED</font>" : "DISARMED");
    m_lblMode->setText(s.flightMode.isEmpty() ? "--" : s.flightMode);
    m_lblLat->setText(QString::number(s.latitudeDeg,       'f', 7));
    m_lblLon->setText(QString::number(s.longitudeDeg,      'f', 7));
    m_lblAlt->setText(QString::number(s.absoluteAltitudeM, 'f', 2));
    m_lblRelAlt->setText(QString::number(s.relativeAltitudeM, 'f', 2));
    m_lblEnuE->setText(QString::number(s.enuEastM,  'f', 3));
    m_lblEnuN->setText(QString::number(s.enuNorthM, 'f', 3));
    m_lblEnuU->setText(QString::number(s.enuUpM,    'f', 3));
    m_lblRoll->setText(QString::number(s.rollDeg,   'f', 2));
    m_lblPitch->setText(QString::number(s.pitchDeg, 'f', 2));
    m_lblYaw->setText(QString::number(s.yawDeg,     'f', 2));
    m_lblBattery->setText(QString::number(s.batteryPercent, 'f', 0) + " %");
    m_lblDist->setText(QString::number(s.distanceToOrigin3dM, 'f', 2));
}

void MainWindow::onConnectionStatusChanged(bool connected, const QString& msg) {
    if (connected) {
        m_statusLabel->setText("● 已连接");
        m_statusLabel->setStyleSheet("color: #50e050; font-weight: bold;");
    } else {
        m_statusLabel->setText("● 未连接");
        m_statusLabel->setStyleSheet("color: #e05050; font-weight: bold;");
    }
    appendLog(msg);
}

void MainWindow::onSourceError(const QString& error) {
    appendLog("<font color='#e05050'>" + error + "</font>");
}

void MainWindow::onLogMessage(const QString& msg) {
    appendLog(msg);
}

void MainWindow::onLogSaved(const QString& path) {
    appendLog(QString("[Log] Saved: %1").arg(path));
    if (m_lblLogPath) m_lblLogPath->setText(QFileInfo(path).fileName());
}

void MainWindow::onFreqTimer() {
    if (!m_lblFreq) return;
    m_freqHz     = m_frameCount;
    m_frameCount = 0;
    m_lblFreq->setText(QString::number(m_freqHz, 'f', 1) + " Hz");
}

void MainWindow::appendLog(const QString& msg) {
    if (!m_logEdit) return;
    QString ts = QDateTime::currentDateTime().toString("HH:mm:ss");
    m_logEdit->append(QString("[%1] %2").arg(ts).arg(msg));
    QTextCursor c = m_logEdit->textCursor();
    c.movePosition(QTextCursor::End);
    m_logEdit->setTextCursor(c);
}

void MainWindow::applyDarkTheme() {
    qApp->setStyle("Fusion");
    QPalette p;
    p.setColor(QPalette::Window,          QColor(28, 28, 32));
    p.setColor(QPalette::WindowText,      QColor(220, 220, 220));
    p.setColor(QPalette::Base,            QColor(22, 22, 26));
    p.setColor(QPalette::AlternateBase,   QColor(35, 35, 40));
    p.setColor(QPalette::Text,            QColor(220, 220, 220));
    p.setColor(QPalette::Button,          QColor(50, 50, 58));
    p.setColor(QPalette::ButtonText,      QColor(220, 220, 220));
    p.setColor(QPalette::Highlight,       QColor(60, 120, 200));
    p.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    qApp->setPalette(p);
}
