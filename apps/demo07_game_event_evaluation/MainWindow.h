#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QListWidget>
#include <QTimer>
#include <QElapsedTimer>
#include <memory>

#include "RenderWidget.h"
#include "mission/MissionConfig.h"
#include "mission/MissionState.h"
#include "event/EventEngine.h"
#include "event/EventTimeline.h"
#include "scoring/ScoreEvaluator.h"
#include "replay/TrajectoryReplay.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void onLoadMission();
    void onStartReplay();
    void onPauseReplay();
    void onResetReplay();
    void onFrameUpdated(const ReplayFrame& frame);
    void onReplayFinished();
    void onFpsUpdated(float fps);
    void onParticleTick();

private:
    void buildUi();
    void buildLeftPanel(QWidget* left);
    void buildRightPanel(QWidget* right);
    void applyDarkTheme();
    void appendLog(const QString& msg);
    void updateScorePanel();
    void loadMissionFromFile(const QString& path);

    // 中央 OpenGL 视口
    RenderWidget* m_renderWidget = nullptr;

    // 左侧控制面板
    QLineEdit*   m_edtMissionPath = nullptr;
    QPushButton* m_btnLoadMission = nullptr;
    QPushButton* m_btnStartReplay = nullptr;
    QPushButton* m_btnPauseReplay = nullptr;
    QPushButton* m_btnResetReplay = nullptr;
    QLabel*      m_lblMissionName = nullptr;
    QLabel*      m_lblTargetCount = nullptr;
    QLabel*      m_lblZoneCount   = nullptr;
    QLabel*      m_lblFrameCount  = nullptr;

    // 右侧统计面板
    QLabel*      m_lblScore       = nullptr;
    QLabel*      m_lblMaxScore    = nullptr;
    QLabel*      m_lblElapsed     = nullptr;
    QLabel*      m_lblBattery     = nullptr;
    QLabel*      m_lblEvents      = nullptr;
    QLabel*      m_lblStatus      = nullptr;
    QLabel*      m_lblFps         = nullptr;
    QListWidget* m_lstEvents      = nullptr;

    // 日志
    QTextEdit*   m_logWidget      = nullptr;

    // 任务数据
    MissionConfig      m_missionConfig;
    MissionState       m_missionState;
    EventEngine        m_eventEngine;
    EventTimeline      m_timeline;
    ScoreEvaluator     m_scoreEvaluator;
    TrajectoryReplay   m_replay;

    // 目标/区域（可变，供 EventEngine 修改）
    std::vector<VirtualTarget> m_targets;
    std::vector<TriggerZone>   m_zones;

    // 粒子更新定时器
    QTimer m_particleTimer;
    float  m_replayElapsed = 0.0f;
    bool   m_missionLoaded = false;
};
