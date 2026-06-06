#include "CoordinateTransformer.h"
#include <cmath>

CoordinateTransformer::CoordinateTransformer()
    : m_hasOrigin(false), m_sceneScale(1.0) {}

void CoordinateTransformer::setOrigin(const Wgs84Coord& origin) {
    m_origin      = origin;
    m_originEcef  = Wgs84Converter::wgs84ToEcef(origin);
    m_hasOrigin   = true;
}

TransformResult CoordinateTransformer::transform(const Wgs84Coord& wgs) const {
    TransformResult result;
    result.wgs84 = wgs;

    if (!m_hasOrigin) {
        result.valid    = false;
        result.errorMsg = "No geo origin set";
        return result;
    }

    // Step 1: WGS84 -> ECEF
    result.ecef = Wgs84Converter::wgs84ToEcef(wgs);

    // Step 2: ECEF -> ENU
    result.enu = Wgs84Converter::ecefToEnu(result.ecef, m_originEcef,
                                            m_origin.lat, m_origin.lon);

    // Step 3: ENU -> SCENE
    result.scene = Wgs84Converter::enuToScene(result.enu, m_sceneScale);

    // Step 4: SCENE -> OpenGL
    result.opengl = Wgs84Converter::sceneToOpenGL(result.scene);

    // 计算距离和方位角
    result.distanceHorizontal = std::sqrt(result.enu.e * result.enu.e +
                                           result.enu.n * result.enu.n);
    result.distanceTotal      = std::sqrt(result.enu.e * result.enu.e +
                                           result.enu.n * result.enu.n +
                                           result.enu.u * result.enu.u);

    // 方位角：从北方向顺时针，atan2(E, N)
    result.azimuth_deg = std::atan2(result.enu.e, result.enu.n) * RAD2DEG;
    if (result.azimuth_deg < 0.0) result.azimuth_deg += 360.0;

    result.valid = true;
    return result;
}

TransformResult CoordinateTransformer::validateOrigin() const {
    if (!m_hasOrigin) {
        TransformResult r;
        r.valid    = false;
        r.errorMsg = "No origin set for validation";
        return r;
    }
    return transform(m_origin);
}
