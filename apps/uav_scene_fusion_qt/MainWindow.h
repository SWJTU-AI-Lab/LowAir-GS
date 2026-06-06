#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>

class RenderWidget;
class UdpJsonReceiver;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onUdpStarted();
    void onUdpStopped();
    void onDataReceived(const QString &uavId, double x, double y, double z, double roll, double pitch, double yaw, double battery);

private:
    void setupUi();

    RenderWidget *m_renderWidget;
    UdpJsonReceiver *m_udpReceiver;

    QLabel *m_statusLabel;
    QLabel *m_uavIdLabel;
    QLabel *m_posLabel;
    QLabel *m_attLabel;
    QLabel *m_batteryLabel;
    QLabel *m_trajCountLabel;
    
    QPushButton *m_startUdpBtn;
    QPushButton *m_stopUdpBtn;
    QPushButton *m_clearTrajBtn;
    QPushButton *m_resetViewBtn;
};

#endif // MAINWINDOW_H
