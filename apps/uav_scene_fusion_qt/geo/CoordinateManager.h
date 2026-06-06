#ifndef COORDINATEMANAGER_H
#define COORDINATEMANAGER_H

#include <QVector3D>

class CoordinateManager
{
public:
    CoordinateManager();

    // SCENE 坐标直接返回
    QVector3D sceneToScene(const QVector3D& p) const;
    
    // WGS84 转 SCENE (预留接口，当前返回默认值)
    QVector3D wgs84ToScene(double lat, double lon, double alt) const;
};

#endif // COORDINATEMANAGER_H
