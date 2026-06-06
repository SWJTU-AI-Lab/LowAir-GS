#pragma once

// MainWindow.h
// Demo-03 主窗口
// 布局：左侧坐标配置面板 | 中间三维视口 | 右侧坐标结果面板 | 底部日志

#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QCheckBox>
#include "RenderWidget.h"
#include "geo/CoordinateTransformer.h"
#include "telemetry/UdpWgs84Receiver.h"
#include "telemetry/GeoUavState.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onLoadConfig();
    void onStartUdp();
    void onStopUdp();
    void onResetOrigin();
    void onClearTrajectory();
    void onResetView();
    void onFitToTrajectory();
    void onTakeScreenshot();
    void onValidateOrigin();

    void onStateReceived(const GeoUavState& state);
    void onUdpLog(const QString& msg);
    void onUdpError(const QString& msg);

private:
    void setupUI();
    void applyDarkTheme();
    void updateCoordPanel(const TransformResult& result);
    void loadConfigFile(const QString& path);
    void applyOriginFromUI();
    void appendLog(const QString& msg);

    // 左侧配置面板
    QLineEdit* m_editOriginLat   = nullptr;
    QLineEdit* m_editOriginLon   = nullptr;
    QLineEdit* m_editOriginAlt   = nullptr;
    QLineEdit* m_editUdpPort     = nullptr;
    QLineEdit* m_editSceneScale  = nullptr;
    QCheckBox* m_cbGrid          = nullptr;
    QCheckBox* m_cbAxis          = nullptr;
    QCheckBox* m_cbOriginMarker  = nullptr;
    QCheckBox* m_cbRings         = nullptr;

    QPushButton* m_btnLoadConfig    = nullptr;
    QPushButton* m_btnStartUdp      = nullptr;
    QPushButton* m_btnStopUdp       = nullptr;
    QPushButton* m_btnResetOrigin   = nullptr;
    QPushButton* m_btnClearTraj     = nullptr;
    QPushButton* m_btnResetView     = nullptr;
    QPushButton* m_btnFitTraj       = nullptr;
    QPushButton* m_btnScreenshot    = nullptr;
    QPushButton* m_btnValidate      = nullptr;

    // 右侧坐标结果面板
    QLabel* m_lblUavId          = nullptr;
    QLabel* m_lblTimestamp      = nullptr;
    QLabel* m_lblRawLat         = nullptr;
    QLabel* m_lblRawLon         = nullptr;
    QLabel* m_lblRawAlt         = nullptr;
    QLabel* m_lblEnuE           = nullptr;
    QLabel* m_lblEnuN           = nullptr;
    QLabel* m_lblEnuU           = nullptr;
    QLabel* m_lblSceneX         = nullptr;
    QLabel* m_lblSceneY         = nullptr;
    QLabel* m_lblSceneZ         = nullptr;
    QLabel* m_lblGlX            = nullptr;
    QLabel* m_lblGlY            = nullptr;
    QLabel* m_lblGlZ            = nullptr;
    QLabel* m_lblDistH          = nullptr;
    QLabel* m_lblDistTotal      = nullptr;
    QLabel* m_lblAzimuth        = nullptr;
    QLabel* m_lblYaw            = nullptr;
    QLabel* m_lblBattery        = nullptr;
    QLabel* m_lblUdpStatus      = nullptr;

    // 底部日志
    QTextEdit* m_logEdit = nullptr;

    // 中间三维视口
    RenderWidget* m_renderWidget = nullptr;

    // 核心对象
    CoordinateTransformer m_transformer;
    UdpWgs84Receiver*     m_receiver = nullptr;

    int m_packetCount = 0;
};
