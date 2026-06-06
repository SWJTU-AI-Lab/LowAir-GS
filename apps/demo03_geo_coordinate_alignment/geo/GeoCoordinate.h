#pragma once

// GeoCoordinate.h
// 地理坐标数据结构定义
// 包含 WGS84、ECEF、ENU、SCENE 四种坐标类型

struct Wgs84Coord {
    double lat = 0.0;  // 纬度 (degree)
    double lon = 0.0;  // 经度 (degree)
    double alt = 0.0;  // 高度 (meter, above ellipsoid)
};

struct EcefCoord {
    double x = 0.0;  // 地心地固 X (meter)
    double y = 0.0;  // 地心地固 Y (meter)
    double z = 0.0;  // 地心地固 Z (meter)
};

struct EnuCoord {
    double e = 0.0;  // East  向东 (meter)
    double n = 0.0;  // North 向北 (meter)
    double u = 0.0;  // Up    向上 (meter)
};

// SCENE 坐标：与 ENU 对齐
// SCENE x = ENU East
// SCENE y = ENU North
// SCENE z = ENU Up
struct SceneCoord {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

// OpenGL 显示坐标（坐标轴适配，非地理转换）
// OpenGL x = SCENE x
// OpenGL y = SCENE z
// OpenGL z = -SCENE y
struct OpenGLCoord {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};
