#include "MainWindow.h"
#include "RenderWidget.h"
#include "telemetry/UdpJsonReceiver.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QDateTime>

static const QString DARK_STYLE = R"(
QMainWindow, QWidget {
    background-color: #2D2D30;
    color: #E0E0E0;
    font-family: "Consolas", "Courier New", monospace;
    font-size: 12px;
}
QGroupBox {
    border: 1px solid #555;
    border-radius: 4px;
    margin-top: 8px;
    padding-top: 8px;
    color: #A0C4FF;
    font-weight: bold;
}
QGroupBox::title {
    subcontrol-origin: margin;
    left: 8px;
    padding: 0 4px;
}
QPushButton {
    background-color: #3C3C3F;
    border: 1px solid #555;
    border-radius: 3px;
    color: #E0E0E0;
    padding: 4px 8px;
    min-height: 28px;
}
QPushButton:hover {
    background-color: #505055;
}
QPushButton:pressed {
    background-color: #2A2A2E;
}
QPushButton:disabled {
    color: #666;
    border-color: #444;
}
QLabel {
    color: #E0E0E0;
}
)";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_udpReceiver(new UdpJsonReceiver(this))
{
    setStyleSheet(DARK_STYLE);
    setupUi();
    
    connect(m_udpReceiver, &UdpJsonReceiver::started, this, &MainWindow::onUdpStarted);
    connect(m_udpReceiver, &UdpJsonReceiver::stopped, this, &MainWindow::onUdpStopped);
    connect(m_udpReceiver, &UdpJsonReceiver::dataReceived, this, &MainWindow::onDataReceived);
    connect(m_udpReceiver, &UdpJsonReceiver::dataReceived, m_renderWidget, &RenderWidget::updateUavState);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUi()
{
    setWindowTitle("LowAir-GS  |  Demo-01: UAV Scene Fusion");

    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(6, 6, 6, 6);
    mainLayout->setSpacing(6);

    // ---- Left Panel ----
    QWidget *leftPanel = new QWidget(this);
    leftPanel->setFixedWidth(240);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(6);

    // Control Group
    QGroupBox *ctrlGroup = new QGroupBox("Control", this);
    QVBoxLayout *ctrlLayout = new QVBoxLayout(ctrlGroup);
    ctrlLayout->setSpacing(4);
    
    m_startUdpBtn = new QPushButton("Start UDP (14580)", this);
    m_stopUdpBtn = new QPushButton("Stop UDP", this);
    m_stopUdpBtn->setEnabled(false);
    m_clearTrajBtn = new QPushButton("Clear Trajectory", this);
    m_resetViewBtn = new QPushButton("Reset View", this);

    ctrlLayout->addWidget(m_startUdpBtn);
    ctrlLayout->addWidget(m_stopUdpBtn);
    ctrlLayout->addWidget(m_clearTrajBtn);
    ctrlLayout->addWidget(m_resetViewBtn);
    
    connect(m_startUdpBtn, &QPushButton::clicked, [this]() {
        m_udpReceiver->startListening(14580);
    });
    connect(m_stopUdpBtn, &QPushButton::clicked, m_udpReceiver, &UdpJsonReceiver::stopListening);
    connect(m_clearTrajBtn, &QPushButton::clicked, [this]() {
        m_renderWidget->clearTrajectory();
        m_trajCountLabel->setText("0");
    });
    connect(m_resetViewBtn, &QPushButton::clicked, [this]() {
        m_renderWidget->resetView();
    });

    // Status Group
    QGroupBox *statusGroup = new QGroupBox("UDP Status", this);
    QFormLayout *statusLayout = new QFormLayout(statusGroup);
    statusLayout->setSpacing(4);
    
    m_statusLabel = new QLabel("Stopped", this);
    m_statusLabel->setStyleSheet("color: #888;");
    statusLayout->addRow("Status:", m_statusLabel);

    // UAV Info Group
    QGroupBox *uavGroup = new QGroupBox("UAV Telemetry", this);
    QFormLayout *uavLayout = new QFormLayout(uavGroup);
    uavLayout->setSpacing(4);
    
    m_uavIdLabel = new QLabel("-", this);
    m_posLabel = new QLabel("-", this);
    m_attLabel = new QLabel("-", this);
    m_batteryLabel = new QLabel("-", this);
    m_trajCountLabel = new QLabel("0", this);

    uavLayout->addRow("UAV ID:", m_uavIdLabel);
    uavLayout->addRow("Position:", m_posLabel);
    uavLayout->addRow("Attitude:", m_attLabel);
    uavLayout->addRow("Battery:", m_batteryLabel);
    uavLayout->addRow("Traj Pts:", m_trajCountLabel);

    // Help Group
    QGroupBox *helpGroup = new QGroupBox("Mouse Control", this);
    QVBoxLayout *helpLayout = new QVBoxLayout(helpGroup);
    QLabel *helpLabel = new QLabel("Left drag: Rotate\nScroll: Zoom", this);
    helpLabel->setStyleSheet("color: #888; font-size: 11px;");
    helpLayout->addWidget(helpLabel);

    leftLayout->addWidget(ctrlGroup);
    leftLayout->addWidget(statusGroup);
    leftLayout->addWidget(uavGroup);
    leftLayout->addWidget(helpGroup);
    leftLayout->addStretch();

    // ---- 3D View ----
    m_renderWidget = new RenderWidget(this);

    mainLayout->addWidget(leftPanel);
    mainLayout->addWidget(m_renderWidget, 1);

    setCentralWidget(centralWidget);
}

void MainWindow::onUdpStarted()
{
    m_startUdpBtn->setEnabled(false);
    m_stopUdpBtn->setEnabled(true);
    m_statusLabel->setText("Listening on :14580");
    m_statusLabel->setStyleSheet("color: #4CAF50;"); // 正常状态绿色
}

void MainWindow::onUdpStopped()
{
    m_startUdpBtn->setEnabled(true);
    m_stopUdpBtn->setEnabled(false);
    m_statusLabel->setText("Stopped");
    m_statusLabel->setStyleSheet("color: #888;");
}

void MainWindow::onDataReceived(const QString &uavId, double x, double y, double z, double roll, double pitch, double yaw, double battery)
{
    m_statusLabel->setText("Receiving Data");
    m_statusLabel->setStyleSheet("color: #4CAF50;");
    
    m_uavIdLabel->setText(uavId);
    m_posLabel->setText(QString::asprintf("%.1f, %.1f, %.1f", x, y, z));
    m_attLabel->setText(QString::asprintf("R%.1f P%.1f Y%.1f", roll, pitch, yaw));
    
    if (battery < 20.0) {
        m_batteryLabel->setStyleSheet("color: #FFEB3B; font-weight: bold;"); // 警告黄色
    } else {
        m_batteryLabel->setStyleSheet("color: #E0E0E0;");
    }
    m_batteryLabel->setText(QString::asprintf("%.1f%%", battery));
    
    m_trajCountLabel->setText(QString::number(m_renderWidget->getTrajectoryPointCount()));
}
