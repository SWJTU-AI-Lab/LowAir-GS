#include "MainWindow.h"
#include <QApplication>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QDateTime>
#include <QScrollBar>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("LowAir-GS  Demo-07  Game Event Evaluation");
    resize(1440, 860);
    applyDarkTheme();
    buildUi();

    // 粒子更新定时器（~30 Hz）
    connect(&m_particleTimer, &QTimer::timeout, this, [this](){
        m_renderWidget->updateParticles(0.033f);
        m_replayElapsed += 0.033f;
        updateScorePanel();
    });
    m_particleTimer.start(33);

    // 连接回放信号
    connect(&m_replay, &TrajectoryReplay::frameUpdated,  this, &MainWindow::onFrameUpdated);
    connect(&m_replay, &TrajectoryReplay::replayFinished, this, &MainWindow::onReplayFinished);

    // 连接事件引擎信号
    connect(&m_eventEngine, &EventEngine::targetTriggered, this, [this](const QString& targetId, const QString& effectType){
        m_lstEvents->addItem(QString("[%1s] 到达目标 %2")
                             .arg(m_replayElapsed, 0, 'f', 1).arg(targetId));
        appendLog(QString("✓ 到达目标 %1 (特效: %2)").arg(targetId).arg(effectType));
        // Label Popup 弹窗
        m_renderWidget->pushPopup(QString("+%1  %2 到达")
                                   .arg(20).arg(targetId));
        // Path Highlight 更新：将当前已记录轨迹点全部标为高亮
        m_renderWidget->setPathHighlight(m_pathPointCount);
        // 触发特效
        for (const auto& t : m_targets) {
            if (t.id == targetId) {
                Effect ge;
                ge.type     = Effect::fromString(effectType);
                ge.position = t.position;
                if (ge.type == EffectType::Explosion) {
                    // 虚拟爆炸视觉特效（非物理、非毁伤模型）
                    ge.color    = QVector4D(1.0f, 0.4f, 0.0f, 1.0f);
                    ge.lifetime = 2.5f;
                    m_renderWidget->pushPopup(QString("[Virtual Explosion Effect] %1").arg(targetId));
                } else {
                    ge.color    = QVector4D(0.2f, 1.0f, 0.3f, 1.0f);
                    ge.lifetime = 2.0f;
                }
                m_renderWidget->spawnEffect(ge);
                break;
            }
        }
        m_renderWidget->setTargets(m_targets);
    });

    connect(&m_eventEngine, &EventEngine::zoneTriggered, this, [this](const QString& zoneId, const QString& effectType){
        m_lstEvents->addItem(QString("[%1s] 进入区域 %2")
                             .arg(m_replayElapsed, 0, 'f', 1).arg(zoneId));
        appendLog(QString("✓ 进入区域 %1 (特效: %2)").arg(zoneId).arg(effectType));
        // Label Popup 弹窗
        m_renderWidget->pushPopup(QString("+%1  进入区域 %2")
                                   .arg(15).arg(zoneId));
        // Pulse 特效
        for (const auto& z : m_zones) {
            if (z.id == zoneId) {
                Effect pe;
                pe.type     = Effect::fromString(effectType);
                pe.position = z.center;
                pe.color    = QVector4D(0.2f, 0.6f, 1.0f, 1.0f);
                pe.radius   = std::max({z.size.x(), z.size.y(), z.size.z()}) * 0.5f;
                pe.lifetime = 1.5f;
                m_renderWidget->spawnEffect(pe);
                break;
            }
        }
        m_renderWidget->setZones(m_zones);
    });
}

void MainWindow::buildUi() {
    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    QHBoxLayout* rootLayout = new QHBoxLayout(central);
    rootLayout->setContentsMargins(4, 4, 4, 4);
    rootLayout->setSpacing(4);

    QWidget* left  = new QWidget;  left->setFixedWidth(240);
    QWidget* right = new QWidget;  right->setFixedWidth(220);

    m_renderWidget = new RenderWidget;
    connect(m_renderWidget, &RenderWidget::fpsUpdated, this, &MainWindow::onFpsUpdated);

    buildLeftPanel(left);
    buildRightPanel(right);

    rootLayout->addWidget(left);
    rootLayout->addWidget(m_renderWidget, 1);
    rootLayout->addWidget(right);
}

void MainWindow::buildLeftPanel(QWidget* left) {
    QVBoxLayout* lay = new QVBoxLayout(left);
    lay->setContentsMargins(4, 4, 4, 4);
    lay->setSpacing(6);

    QGroupBox* grpLoad = new QGroupBox("任务配置");
    QVBoxLayout* loadLay = new QVBoxLayout(grpLoad);
    m_edtMissionPath = new QLineEdit;
    m_edtMissionPath->setPlaceholderText("mission_config.json 路径");
    m_btnLoadMission = new QPushButton("Load Mission");
    loadLay->addWidget(m_edtMissionPath);
    loadLay->addWidget(m_btnLoadMission);
    connect(m_btnLoadMission, &QPushButton::clicked, this, &MainWindow::onLoadMission);

    QGroupBox* grpInfo = new QGroupBox("任务信息");
    QVBoxLayout* infoLay = new QVBoxLayout(grpInfo);
    m_lblMissionName = new QLabel("名称: --");
    m_lblTargetCount = new QLabel("目标数: --");
    m_lblZoneCount   = new QLabel("区域数: --");
    m_lblFrameCount  = new QLabel("帧数: --");
    infoLay->addWidget(m_lblMissionName);
    infoLay->addWidget(m_lblTargetCount);
    infoLay->addWidget(m_lblZoneCount);
    infoLay->addWidget(m_lblFrameCount);

    QGroupBox* grpReplay = new QGroupBox("回放控制");
    QVBoxLayout* repLay = new QVBoxLayout(grpReplay);
    m_btnStartReplay = new QPushButton("▶ Start Replay");
    m_btnPauseReplay = new QPushButton("⏸ Pause");
    m_btnResetReplay = new QPushButton("⏹ Reset");
    m_btnStartReplay->setEnabled(false);
    m_btnPauseReplay->setEnabled(false);
    m_btnResetReplay->setEnabled(false);
    repLay->addWidget(m_btnStartReplay);
    repLay->addWidget(m_btnPauseReplay);
    repLay->addWidget(m_btnResetReplay);
    connect(m_btnStartReplay, &QPushButton::clicked, this, &MainWindow::onStartReplay);
    connect(m_btnPauseReplay, &QPushButton::clicked, this, &MainWindow::onPauseReplay);
    connect(m_btnResetReplay, &QPushButton::clicked, this, &MainWindow::onResetReplay);

    QGroupBox* grpLog = new QGroupBox("日志");
    QVBoxLayout* logLay = new QVBoxLayout(grpLog);
    m_logWidget = new QTextEdit;
    m_logWidget->setReadOnly(true);
    m_logWidget->setMaximumHeight(160);
    logLay->addWidget(m_logWidget);

    lay->addWidget(grpLoad);
    lay->addWidget(grpInfo);
    lay->addWidget(grpReplay);
    lay->addWidget(grpLog);
    lay->addStretch();
}

void MainWindow::buildRightPanel(QWidget* right) {
    QVBoxLayout* lay = new QVBoxLayout(right);
    lay->setContentsMargins(4, 4, 4, 4);
    lay->setSpacing(6);

    QGroupBox* grpScore = new QGroupBox("评分统计");
    QVBoxLayout* scoreLay = new QVBoxLayout(grpScore);
    m_lblScore    = new QLabel("得分: 0");
    m_lblMaxScore = new QLabel("满分: 0");
    m_lblElapsed  = new QLabel("用时: 00:00");
    m_lblBattery  = new QLabel("电量: 100%");
    m_lblEvents   = new QLabel("事件数: 0");
    m_lblStatus   = new QLabel("状态: Idle");
    m_lblFps      = new QLabel("FPS: --");
    scoreLay->addWidget(m_lblScore);
    scoreLay->addWidget(m_lblMaxScore);
    scoreLay->addWidget(m_lblElapsed);
    scoreLay->addWidget(m_lblBattery);
    scoreLay->addWidget(m_lblEvents);
    scoreLay->addWidget(m_lblStatus);
    scoreLay->addWidget(m_lblFps);

    QGroupBox* grpEvents = new QGroupBox("事件记录");
    QVBoxLayout* evtLay = new QVBoxLayout(grpEvents);
    m_lstEvents = new QListWidget;
    m_lstEvents->setMaximumHeight(240);
    evtLay->addWidget(m_lstEvents);

    lay->addWidget(grpScore);
    lay->addWidget(grpEvents);
    lay->addStretch();
}

void MainWindow::applyDarkTheme() {
    qApp->setStyleSheet(R"(
        QWidget { background:#1a1b22; color:#e0e0e0; font-size:12px; }
        QGroupBox { border:1px solid #3a3b4a; border-radius:4px; margin-top:8px; padding-top:6px; }
        QGroupBox::title { subcontrol-origin:margin; left:8px; color:#8090c0; }
        QPushButton { background:#2a3050; border:1px solid #4050a0; border-radius:3px; padding:4px 8px; }
        QPushButton:hover { background:#3a4070; }
        QPushButton:disabled { background:#1a1b22; color:#505060; border-color:#303040; }
        QLineEdit { background:#0e0f14; border:1px solid #3a3b4a; border-radius:3px; padding:3px; }
        QTextEdit { background:#0e0f14; border:1px solid #3a3b4a; }
        QListWidget { background:#0e0f14; border:1px solid #3a3b4a; }
        QLabel { color:#c0c8e0; }
    )");
}

void MainWindow::appendLog(const QString& msg) {
    QString ts = QDateTime::currentDateTime().toString("hh:mm:ss");
    m_logWidget->append(QString("[%1] %2").arg(ts, msg));
    m_logWidget->verticalScrollBar()->setValue(m_logWidget->verticalScrollBar()->maximum());
}

void MainWindow::onLoadMission() {
    QString path = m_edtMissionPath->text().trimmed();
    if (path.isEmpty()) {
        path = QFileDialog::getOpenFileName(this, "选择任务配置文件", "",
                                             "JSON (*.json);;All (*.*)");
        if (path.isEmpty()) return;
        m_edtMissionPath->setText(path);
    }
    loadMissionFromFile(path);
}

void MainWindow::loadMissionFromFile(const QString& path) {
    MissionConfig cfg;
    if (!MissionConfig::loadFromFile(path, cfg)) {
        QMessageBox::warning(this, "加载失败", "无法解析任务配置文件：" + path);
        appendLog("ERROR: 加载任务失败 " + path);
        return;
    }
    m_missionConfig = cfg;

    // 复制目标和区域
    m_targets = m_missionConfig.targets;
    m_zones   = m_missionConfig.zones;

    // 重置任务状态
    m_missionState = MissionState();
    m_missionState.missionName    = m_missionConfig.scenarioName;
    m_missionState.totalTargets   = (int)m_targets.size();
    m_missionState.totalZones     = (int)m_zones.size();
    m_missionState.maxScore       = m_missionConfig.maxScore;
    m_missionState.status         = MissionStatus::Idle;

    // 重置事件时间线
    m_timeline.clear();

    // 配置事件引擎
    m_eventEngine.reset();
    m_eventEngine.setTargets(&m_targets);
    m_eventEngine.setZones(&m_zones);
    m_eventEngine.setTimeline(&m_timeline);

    // 配置评分器
    m_scoreEvaluator.setConfig(&m_missionConfig);
    m_scoreEvaluator.setTimeline(&m_timeline);

    // 加载轨迹回放
    m_replay.reset();
    m_replay.setLoop(false);
    if (!m_missionConfig.replayTrackPath.isEmpty()) {
        QString trackPath = m_missionConfig.replayTrackPath;
        // 相对路径：相对于仓库根目录（JSON 文件所在目录的父父目录）
        if (!QFileInfo(trackPath).isAbsolute()) {
            QFileInfo jsonFi(path);
            // 先尝试相对于 JSON 文件所在目录
            QString relToJson = jsonFi.absoluteDir().absoluteFilePath(trackPath);
            if (QFileInfo::exists(relToJson)) {
                trackPath = relToJson;
            } else {
                // 再尝试相对于仓库根目录（JSON 文件所在目录的父目录）
                QString repoRoot = jsonFi.absoluteDir().absoluteFilePath("../..");
                trackPath = QDir(repoRoot).absoluteFilePath(trackPath);
            }
        }
        m_replay.loadTrack(trackPath);
    }

    // 更新视口
    m_renderWidget->setTargets(m_targets);
    m_renderWidget->setZones(m_zones);
    m_renderWidget->clearTrajectory();

    // 更新信息面板
    m_lblMissionName->setText("名称: " + m_missionConfig.scenarioName);
    m_lblTargetCount->setText(QString("目标数: %1").arg(m_targets.size()));
    m_lblZoneCount->setText(QString("区域数: %1").arg(m_zones.size()));
    m_lblFrameCount->setText(QString("帧数: %1").arg(m_replay.frameCount()));

    m_btnStartReplay->setEnabled(true);
    m_btnResetReplay->setEnabled(true);
    m_missionLoaded  = true;
    m_replayElapsed  = 0.0f;
    m_pathPointCount = 0;
    m_lstEvents->clear();
    updateScorePanel();

    appendLog("任务加载成功: " + m_missionConfig.scenarioName);
    appendLog(QString("  目标 %1 个，区域 %2 个，轨迹帧 %3 帧")
              .arg(m_targets.size()).arg(m_zones.size()).arg(m_replay.frameCount()));
}

void MainWindow::onStartReplay() {
    if (!m_missionLoaded) return;
    m_missionState.status = MissionStatus::Running;
    m_missionState.startTime = QDateTime::currentDateTime();
    m_replay.start();
    m_btnStartReplay->setEnabled(false);
    m_btnPauseReplay->setEnabled(true);
    updateScorePanel();
    appendLog("回放开始");
}

void MainWindow::onPauseReplay() {
    if (m_replay.isPlaying()) {
        m_replay.pause();
        m_missionState.status = MissionStatus::Paused;
        m_btnPauseReplay->setText("▶ Resume");
        appendLog("回放暂停");
    } else {
        m_replay.start();
        m_missionState.status = MissionStatus::Running;
        m_btnPauseReplay->setText("⏸ Pause");
        appendLog("回放继续");
    }
    updateScorePanel();
}

void MainWindow::onResetReplay() {
    m_replay.reset();
    m_missionState = MissionState();
    m_missionState.missionName  = m_missionConfig.scenarioName;
    m_missionState.totalTargets = (int)m_targets.size();
    m_missionState.totalZones   = (int)m_zones.size();
    m_missionState.maxScore     = m_missionConfig.maxScore;

    // 重置目标/区域状态
    for (auto& t : m_targets) { t.triggered = false; t.completed = false; }
    for (auto& z : m_zones)   { z.triggered = false; z.completed = false; }
    m_timeline.clear();
    m_eventEngine.reset();

    m_renderWidget->setTargets(m_targets);
    m_renderWidget->setZones(m_zones);
    m_renderWidget->clearTrajectory();
    m_lstEvents->clear();
    m_replayElapsed  = 0.0f;
    m_pathPointCount = 0;
    m_btnStartReplay->setEnabled(true);
    m_btnPauseReplay->setEnabled(false);
    m_btnPauseReplay->setText("⏸ Pause");
    updateScorePanel();
    appendLog("回放已重置");
}

void MainWindow::onFrameUpdated(const ReplayFrame& frame) {
    m_replayElapsed = frame.timestamp;
    m_missionState.elapsedSec   = frame.timestamp;
    m_missionState.batteryEstimate = frame.battery;

    // 更新无人机位置
    m_renderWidget->setUavPosition(frame.position, frame.yawDeg);
    m_pathPointCount++;  // 跟踪轨迹点计数（用于 Path Highlight）

    // 驱动事件引擎检测
    m_eventEngine.update(frame.position, frame.timestamp);

    // 更新已完成计数
    int tc = 0, zc = 0;
    for (const auto& t : m_targets) if (t.completed) tc++;
    for (const auto& z : m_zones)   if (z.completed) zc++;
    m_missionState.targetsCompleted = tc;
    m_missionState.zonesCompleted   = zc;
    m_missionState.eventsTriggered  = m_timeline.count();

    // 实时评分
    m_scoreEvaluator.evaluate(m_missionState);
    m_missionState.currentScore = m_scoreEvaluator.totalScore();

    // 更新 HUD
    m_renderWidget->setHudData(
        m_scoreEvaluator.totalScore(),
        m_scoreEvaluator.maxScore(),
        frame.timestamp,
        frame.battery,
        m_timeline.count(),
        m_missionState.statusString());
}

void MainWindow::onReplayFinished() {
    m_missionState.status = MissionStatus::Completed;
    m_missionState.endTime = QDateTime::currentDateTime();
    m_btnPauseReplay->setEnabled(false);
    m_btnStartReplay->setEnabled(false);

    // 最终评分
    m_scoreEvaluator.evaluate(m_missionState);
    appendLog(QString("回放结束 — 最终得分: %1 / %2")
              .arg(m_scoreEvaluator.totalScore())
              .arg(m_scoreEvaluator.maxScore()));

    // 粒子庆祝特效
    Effect e;
    e.type     = EffectType::Particle;
    e.position = QVector3D(0, 5, 0);
    e.lifetime = 3.0f;
    m_renderWidget->spawnEffect(e);
    updateScorePanel();
}

void MainWindow::onParticleTick() {
    // 由 m_particleTimer 驱动，已在构造函数中连接
}

void MainWindow::updateScorePanel() {
    int mins = (int)(m_replayElapsed / 60);
    int secs = (int)(m_replayElapsed) % 60;

    m_lblScore->setText(QString("得分: %1").arg(m_scoreEvaluator.totalScore()));
    m_lblMaxScore->setText(QString("满分: %1").arg(m_scoreEvaluator.maxScore()));
    m_lblElapsed->setText(QString("用时: %1:%2")
                          .arg(mins, 2, 10, QChar('0'))
                          .arg(secs, 2, 10, QChar('0')));
    m_lblBattery->setText(QString("电量: %1%").arg((int)m_missionState.batteryEstimate));
    m_lblEvents->setText(QString("事件数: %1").arg(m_timeline.count()));
    m_lblStatus->setText("状态: " + m_missionState.statusString());
}

void MainWindow::onFpsUpdated(float fps) {
    m_lblFps->setText(QString("FPS: %1").arg(fps, 0, 'f', 1));
}
