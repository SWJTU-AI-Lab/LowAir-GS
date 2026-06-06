#pragma once
#include <QString>

// 统一无人机遥测状态结构体
// 承载从 MAVLink / MAVSDK / ROS2 等来源解析后的完整遥测帧
struct DroneTelemetryState {
    QString vehicleId;
    qint64  timestampMs = 0;

    bool connected  = false;
    bool gpsValid   = false;
    bool armed      = false;

    QString sourceType;   // MAVSDK / MAVLinkRaw / ROS2 / Replay
    QString flightMode;

    // WGS84 全局位置
    double latitudeDeg        = 0.0;
    double longitudeDeg       = 0.0;
    double absoluteAltitudeM  = 0.0;
    double relativeAltitudeM  = 0.0;

    // ENU 局部坐标（以 geo_origin 为参考原点，单位：米）
    double enuEastM   = 0.0;
    double enuNorthM  = 0.0;
    double enuUpM     = 0.0;

    // SCENE 坐标（与 ENU 对齐，x=East, y=North, z=Up）
    double sceneX = 0.0;
    double sceneY = 0.0;
    double sceneZ = 0.0;

    // OpenGL 显示坐标（x=scene.x, y=scene.z, z=-scene.y）
    double openglX = 0.0;
    double openglY = 0.0;
    double openglZ = 0.0;

    // 姿态（欧拉角，单位：度）
    double rollDeg  = 0.0;
    double pitchDeg = 0.0;
    double yawDeg   = 0.0;

    // NED 速度（单位：m/s）
    double velocityNorthMps = 0.0;
    double velocityEastMps  = 0.0;
    double velocityDownMps  = 0.0;

    // 电量（百分比）
    double batteryPercent = 0.0;

    // 距离计算
    double horizontalDistanceToOriginM = 0.0;
    double distanceToOrigin3dM         = 0.0;
};
