#pragma once
#include <QVector3D>

// 单帧回放数据（无人机位置、姿态、时间戳）
struct ReplayFrame {
    float     timestamp = 0.0f;   // 相对时间（秒）
    QVector3D position;
    float     yawDeg    = 0.0f;
    float     pitchDeg  = 0.0f;
    float     rollDeg   = 0.0f;
    float     battery   = 100.0f; // 电量百分比
};
