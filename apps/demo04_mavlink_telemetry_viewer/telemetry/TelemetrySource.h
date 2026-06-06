#pragma once
#include <QObject>
#include <QString>
#include "DroneTelemetryState.h"

// 遥测数据源抽象接口
// 支持后续扩展：MAVSDK、MAVLink Raw、ROS2、日志回放
// Demo-04 第一版实现：MavlinkRawTelemetrySource（MAVLink UDP 原始消息解析）
class TelemetrySource : public QObject {
    Q_OBJECT

public:
    explicit TelemetrySource(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~TelemetrySource() = default;

    // 启动连接，connectionUrl 示例：
    //   udp://:14550   (MAVLink UDP 监听)
    //   tcp://127.0.0.1:5760
    //   serial:///dev/ttyUSB0:57600
    virtual bool start(const QString& connectionUrl) = 0;
    virtual void stop() = 0;
    virtual bool isRunning() const = 0;
    virtual QString name() const = 0;

signals:
    // 收到新遥测帧时触发
    void telemetryUpdated(const DroneTelemetryState& state);
    // 连接状态变化时触发
    void connectionStatusChanged(bool connected, const QString& message);
    // 错误时触发
    void errorOccurred(const QString& error);
    // 日志消息
    void logMessage(const QString& msg);
};
