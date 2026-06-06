#pragma once

// CoordinateTransformer.h
// 完整坐标转换链路管理器
// WGS84 -> ECEF -> ENU -> SCENE -> OpenGL

#include "GeoCoordinate.h"
#include "Wgs84Converter.h"
#include <QString>

struct TransformResult {
    Wgs84Coord  wgs84;
    EcefCoord   ecef;
    EnuCoord    enu;
    SceneCoord  scene;
    OpenGLCoord opengl;

    double distanceHorizontal = 0.0;  // 水平距离 (m)
    double distanceTotal      = 0.0;  // 三维距离 (m)
    double azimuth_deg        = 0.0;  // 方位角 (degree, 0=North, 90=East)

    bool valid = false;
    QString errorMsg;
};

class CoordinateTransformer {
public:
    CoordinateTransformer();

    // 设置地理参考原点
    void setOrigin(const Wgs84Coord& origin);
    Wgs84Coord getOrigin() const { return m_origin; }
    bool hasOrigin() const { return m_hasOrigin; }

    // 设置场景缩放比例
    void setSceneScale(double scale) { m_sceneScale = scale; }
    double getSceneScale() const { return m_sceneScale; }

    // 完整转换链路：WGS84 -> ECEF -> ENU -> SCENE -> OpenGL
    TransformResult transform(const Wgs84Coord& wgs) const;

    // 固定测试点验证（原点应返回 ENU ≈ (0,0,0)）
    TransformResult validateOrigin() const;

private:
    Wgs84Coord m_origin;
    EcefCoord  m_originEcef;
    bool       m_hasOrigin = false;
    double     m_sceneScale = 1.0;

    static constexpr double RAD2DEG = 180.0 / 3.14159265358979323846;
};
