#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QGroupBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QTimer>
#include <QDateTime>
#include "RenderWidget.h"
#include "telemetry/MavlinkRawTelemetrySource.h"
#include "telemetry/TelemetryRecorder.h"
#include "telemetry/DroneTelemetryState.h"
#include "geo/GeoCoordinate.h"
#include "geo/CoordinateTransformer.h"

// Demo-04 主窗口
// 布局：左侧控制面板（连接配置 + 原点配置 + 遥测状态 + 日志）| 右侧 OpenGL 三维视口
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onConnectClicked();
    void onDisconnectClicked();
    void onClearTrajectoryClicked();
    void onResetViewClicked();
    void onStartLogClicked();
    void onStopLogClicked();
    void onTelemetryUpdated(const DroneTelemetryState& state);
    void onConnectionStatusChanged(bool connected, const QString& msg);
    void onSourceError(const QString& error);
    void onLogMessage(const QString& msg);
    void onLogSaved(const QString& path);
    void onFreqTimer();

private:
    void buildUi();
    void updateCoordPanel(const DroneTelemetryState& s);
    void appendLog(const QString& msg);
    void applyDarkTheme();

    // 左侧面板控件
    QLineEdit*      m_urlEdit       = nullptr;
    QDoubleSpinBox* m_originLat     = nullptr;
    QDoubleSpinBox* m_originLon     = nullptr;
    QDoubleSpinBox* m_originAlt     = nullptr;
    QPushButton*    m_connectBtn    = nullptr;
    QPushButton*    m_disconnectBtn = nullptr;
    QPushButton*    m_startLogBtn   = nullptr;
    QPushButton*    m_stopLogBtn    = nullptr;
    QPushButton*    m_clearTrajBtn  = nullptr;
    QPushButton*    m_resetViewBtn  = nullptr;
    QLabel*         m_statusLabel   = nullptr;

    // 遥测状态面板
    QLabel* m_lblConnected  = nullptr;
    QLabel* m_lblGps        = nullptr;
    QLabel* m_lblArmed      = nullptr;
    QLabel* m_lblMode       = nullptr;
    QLabel* m_lblLat        = nullptr;
    QLabel* m_lblLon        = nullptr;
    QLabel* m_lblAlt        = nullptr;
    QLabel* m_lblRelAlt     = nullptr;
    QLabel* m_lblEnuE       = nullptr;
    QLabel* m_lblEnuN       = nullptr;
    QLabel* m_lblEnuU       = nullptr;
    QLabel* m_lblRoll       = nullptr;
    QLabel* m_lblPitch      = nullptr;
    QLabel* m_lblYaw        = nullptr;
    QLabel* m_lblBattery    = nullptr;
    QLabel* m_lblDist       = nullptr;
    QLabel* m_lblFreq       = nullptr;
    QLabel* m_lblLogPath    = nullptr;

    // 日志窗口
    QTextEdit* m_logEdit = nullptr;

    // 右侧 OpenGL 视口
    RenderWidget* m_renderWidget = nullptr;

    // 遥测源 + 记录器
    MavlinkRawTelemetrySource* m_source   = nullptr;
    TelemetryRecorder*         m_recorder = nullptr;
    CoordinateTransformer      m_transformer;
    Wgs84Coord                 m_geoOrigin;

    // 遥测频率统计
    QTimer* m_freqTimer   = nullptr;
    int     m_frameCount  = 0;
    double  m_freqHz      = 0.0;
};
