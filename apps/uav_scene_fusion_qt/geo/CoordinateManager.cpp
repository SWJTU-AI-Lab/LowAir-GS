#include "CoordinateManager.h"

CoordinateManager::CoordinateManager()
{
}

QVector3D CoordinateManager::sceneToRender(const QVector3D& p) const
{
    // Demo-01：SCENE 局部演示坐标直接透传，不做任何转换
    return p;
}

QVector3D CoordinateManager::wgs84ToScene(double lat, double lon, double alt) const
{
    // Reserved for Demo-03：WGS84 → ENU → SCENE 真实地理坐标转换
    // Demo-01 不使用此接口
    Q_UNUSED(lat);
    Q_UNUSED(lon);
    Q_UNUSED(alt);
    return QVector3D(0, 0, 0);
}
