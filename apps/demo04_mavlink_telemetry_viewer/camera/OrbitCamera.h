#pragma once
#include <QMatrix4x4>
#include <QVector3D>

// 轨道相机（球面坐标系）
// 支持：鼠标左键旋转、右键平移、滚轮缩放、重置、自适应轨迹半径
class OrbitCamera {
public:
    OrbitCamera();

    QMatrix4x4 viewMatrix() const;

    void mouseRotate(float dx, float dy);
    void mousePan(float dx, float dy);
    void mouseZoom(float delta);
    void reset();
    void fitToRadius(float radius);

    float azimuth()   const { return m_azimuth; }
    float elevation() const { return m_elevation; }
    float distance()  const { return m_distance; }

private:
    float    m_azimuth   = 30.0f;
    float    m_elevation = 40.0f;
    float    m_distance  = 200.0f;
    QVector3D m_target   = {0.0f, 0.0f, 0.0f};
};
