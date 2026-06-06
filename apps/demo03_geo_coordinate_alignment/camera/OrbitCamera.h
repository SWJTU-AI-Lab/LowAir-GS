#pragma once

// OrbitCamera.h
// 轨道相机：支持旋转、缩放、平移，适用于 ENU 局部场景浏览

#include <QMatrix4x4>
#include <QVector3D>
#include <QPoint>

class OrbitCamera {
public:
    OrbitCamera();

    // 获取视图矩阵
    QMatrix4x4 viewMatrix() const;
    // 获取投影矩阵
    QMatrix4x4 projectionMatrix(float aspect) const;

    // 鼠标交互
    void mousePressEvent(Qt::MouseButton button, const QPoint& pos);
    void mouseMoveEvent(const QPoint& pos);
    void mouseReleaseEvent();
    void wheelEvent(float delta);

    // 重置视图
    void reset();
    // 适配到指定半径
    void fitToRadius(float radius);

    float distance() const { return m_distance; }
    void setDistance(float d) { m_distance = d; }

private:
    float   m_azimuth   = 30.0f;    // 水平旋转角 (degree)
    float   m_elevation = 40.0f;    // 仰角 (degree)
    float   m_distance  = 150.0f;   // 相机到目标距离 (meter)
    QVector3D m_target  = {0, 0, 0};

    Qt::MouseButton m_pressedButton = Qt::NoButton;
    QPoint          m_lastPos;

    float m_fov    = 45.0f;
    float m_zNear  = 0.1f;
    float m_zFar   = 5000.0f;
};
