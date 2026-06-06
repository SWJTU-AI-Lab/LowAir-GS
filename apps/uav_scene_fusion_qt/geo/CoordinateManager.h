#ifndef COORDINATEMANAGER_H
#define COORDINATEMANAGER_H

#include <QVector3D>

/**
 * @brief CoordinateManager — 坐标管理（预留接口）
 *
 * Demo-01 坐标约定：
 *   输入坐标为 SCENE 局部演示坐标（单位：米），由 Python 模拟器生成。
 *   x/y/z 直接用于演示场景，不进行任何真实地理坐标转换。
 *
 * 后续扩展（Reserved for Demo-03）：
 *   WGS84（经纬高）→ ENU（东北天）→ SCENE（局部场景坐标）
 *   届时将在 Demo-03 中实现 wgs84ToEnu() 和 enuToScene() 方法。
 */
class CoordinateManager
{
public:
    CoordinateManager();

    /**
     * @brief SCENE 坐标直接透传（Demo-01 使用）
     * @param p  SCENE 局部演示坐标（x 向右，y 向前，z 向上）
     * @return   原样返回，不做任何转换
     */
    QVector3D sceneToRender(const QVector3D& p) const;

    /**
     * @brief WGS84 转 SCENE（Reserved for Demo-03）
     * @note  Demo-01 不使用此接口，当前返回零向量
     */
    QVector3D wgs84ToScene(double lat, double lon, double alt) const;
};

#endif // COORDINATEMANAGER_H
