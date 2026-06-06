#pragma once

// GeoUavState.h
// WGS84 格式无人机遥测状态结构体
// UDP JSON 格式：{"uav_id":"geo_sim_01","timestamp_ms":...,"frame":"WGS84",
//                 "lat":30.7,"lon":104.0,"alt":520.0,
//                 "roll":0.0,"pitch":0.0,"yaw":90.0,"battery":88.0}

#include <QString>

struct GeoUavState {
    QString uavId;
    qint64  timestampMs = 0;
    QString frame;        // 应为 "WGS84"

    double lat     = 0.0;  // 纬度 (degree)
    double lon     = 0.0;  // 经度 (degree)
    double alt     = 0.0;  // 高度 (meter)

    double roll    = 0.0;  // 横滚角 (degree)
    double pitch   = 0.0;  // 俯仰角 (degree)
    double yaw     = 0.0;  // 偏航角 (degree, 0=North, 90=East)

    double battery = 0.0;  // 电量百分比

    bool valid = false;
};
