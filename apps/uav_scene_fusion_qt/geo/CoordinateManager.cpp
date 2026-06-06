#include "CoordinateManager.h"

CoordinateManager::CoordinateManager()
{
}

QVector3D CoordinateManager::sceneToScene(const QVector3D& p) const
{
    return p;
}

QVector3D CoordinateManager::wgs84ToScene(double lat, double lon, double alt) const
{
    // TODO: 实现 WGS84 到 ENU，再到 SCENE 的转换
    Q_UNUSED(lat);
    Q_UNUSED(lon);
    Q_UNUSED(alt);
    return QVector3D(0, 0, 0);
}
