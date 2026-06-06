#pragma once

// Wgs84Converter.h
// WGS84 椭球参数与基础转换函数
// 实现 WGS84 -> ECEF -> ENU 标准公式
// 精度适用于局部场景演示（半径 < 500m）
// 后续工程版本建议切换到 GeographicLib 或 PROJ

#include "GeoCoordinate.h"

class Wgs84Converter {
public:
    // WGS84 椭球参数
    // 长半轴 a = 6378137.0 m
    // 扁率倒数 1/f = 298.257223563
    static constexpr double WGS84_A = 6378137.0;
    static constexpr double WGS84_F = 1.0 / 298.257223563;
    static constexpr double WGS84_B = WGS84_A * (1.0 - WGS84_F);
    static constexpr double WGS84_E2 = 2.0 * WGS84_F - WGS84_F * WGS84_F;  // 第一偏心率平方

    // WGS84 -> ECEF
    // 公式：
    //   N(φ) = a / sqrt(1 - e² sin²φ)
    //   X = (N + h) cosφ cosλ
    //   Y = (N + h) cosφ sinλ
    //   Z = (N(1-e²) + h) sinφ
    static EcefCoord wgs84ToEcef(const Wgs84Coord& wgs);

    // ECEF -> ENU（以参考原点为中心）
    // 公式：
    //   dX = X - X0, dY = Y - Y0, dZ = Z - Z0
    //   E = -sinλ₀ dX + cosλ₀ dY
    //   N = -sinφ₀ cosλ₀ dX - sinφ₀ sinλ₀ dY + cosφ₀ dZ
    //   U =  cosφ₀ cosλ₀ dX + cosφ₀ sinλ₀ dY + sinφ₀ dZ
    static EnuCoord ecefToEnu(const EcefCoord& ecef,
                               const EcefCoord& originEcef,
                               double originLat_deg,
                               double originLon_deg);

    // 一步完成 WGS84 -> ENU
    static EnuCoord wgs84ToEnu(const Wgs84Coord& wgs,
                                const Wgs84Coord& origin);

    // ENU -> SCENE（直接映射，scale = 1.0 时等价）
    static SceneCoord enuToScene(const EnuCoord& enu, double sceneScale = 1.0);

    // SCENE -> OpenGL 显示坐标（坐标轴适配）
    static OpenGLCoord sceneToOpenGL(const SceneCoord& scene);

private:
    static constexpr double DEG2RAD = 3.14159265358979323846 / 180.0;
};
