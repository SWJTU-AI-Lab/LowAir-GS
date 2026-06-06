#include "Wgs84Converter.h"
#include <cmath>

// WGS84 -> ECEF
// 参考：NIMA TR8350.2, WGS84 Technical Report
EcefCoord Wgs84Converter::wgs84ToEcef(const Wgs84Coord& wgs) {
    const double lat = wgs.lat * DEG2RAD;
    const double lon = wgs.lon * DEG2RAD;
    const double alt = wgs.alt;

    // 卯酉圈曲率半径 N(φ) = a / sqrt(1 - e² sin²φ)
    const double sinLat = std::sin(lat);
    const double cosLat = std::cos(lat);
    const double sinLon = std::sin(lon);
    const double cosLon = std::cos(lon);

    const double N = WGS84_A / std::sqrt(1.0 - WGS84_E2 * sinLat * sinLat);

    EcefCoord ecef;
    ecef.x = (N + alt) * cosLat * cosLon;
    ecef.y = (N + alt) * cosLat * sinLon;
    ecef.z = (N * (1.0 - WGS84_E2) + alt) * sinLat;
    return ecef;
}

// ECEF -> ENU（以参考原点为中心）
EnuCoord Wgs84Converter::ecefToEnu(const EcefCoord& ecef,
                                    const EcefCoord& originEcef,
                                    double originLat_deg,
                                    double originLon_deg) {
    const double lat0 = originLat_deg * DEG2RAD;
    const double lon0 = originLon_deg * DEG2RAD;

    const double sinLat0 = std::sin(lat0);
    const double cosLat0 = std::cos(lat0);
    const double sinLon0 = std::sin(lon0);
    const double cosLon0 = std::cos(lon0);

    // ECEF 差向量
    const double dx = ecef.x - originEcef.x;
    const double dy = ecef.y - originEcef.y;
    const double dz = ecef.z - originEcef.z;

    // ENU 旋转矩阵
    // E = -sinλ₀ dx + cosλ₀ dy
    // N = -sinφ₀ cosλ₀ dx - sinφ₀ sinλ₀ dy + cosφ₀ dz
    // U =  cosφ₀ cosλ₀ dx + cosφ₀ sinλ₀ dy + sinφ₀ dz
    EnuCoord enu;
    enu.e = -sinLon0 * dx + cosLon0 * dy;
    enu.n = -sinLat0 * cosLon0 * dx - sinLat0 * sinLon0 * dy + cosLat0 * dz;
    enu.u =  cosLat0 * cosLon0 * dx + cosLat0 * sinLon0 * dy + sinLat0 * dz;
    return enu;
}

// WGS84 -> ENU（一步完成）
EnuCoord Wgs84Converter::wgs84ToEnu(const Wgs84Coord& wgs, const Wgs84Coord& origin) {
    const EcefCoord ecef = wgs84ToEcef(wgs);
    const EcefCoord originEcef = wgs84ToEcef(origin);
    return ecefToEnu(ecef, originEcef, origin.lat, origin.lon);
}

// ENU -> SCENE（直接映射）
// SCENE x = ENU East, SCENE y = ENU North, SCENE z = ENU Up
SceneCoord Wgs84Converter::enuToScene(const EnuCoord& enu, double sceneScale) {
    SceneCoord scene;
    scene.x = static_cast<float>(enu.e * sceneScale);
    scene.y = static_cast<float>(enu.n * sceneScale);
    scene.z = static_cast<float>(enu.u * sceneScale);
    return scene;
}

// SCENE -> OpenGL 显示坐标（坐标轴适配，非地理转换）
// OpenGL x = SCENE x (East)
// OpenGL y = SCENE z (Up)
// OpenGL z = -SCENE y (-North)
OpenGLCoord Wgs84Converter::sceneToOpenGL(const SceneCoord& scene) {
    OpenGLCoord gl;
    gl.x =  scene.x;
    gl.y =  scene.z;
    gl.z = -scene.y;
    return gl;
}
