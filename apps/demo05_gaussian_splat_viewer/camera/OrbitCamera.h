#pragma once
#include <QMatrix4x4>
#include <QVector3D>

// 轨道相机：支持旋转（Orbit）、缩放（Zoom）、平移（Pan）
class OrbitCamera {
public:
    OrbitCamera();

    void setTarget(const QVector3D &target);
    void setDistance(float dist);
    void setAspect(float aspect);

    void orbit(float dYaw, float dPitch);
    void zoom(float delta);
    void pan(float dx, float dy);
    void reset();

    QMatrix4x4 viewMatrix() const;
    QMatrix4x4 projMatrix() const;
    QMatrix4x4 mvpMatrix() const;
    QVector3D position() const;

    float fovDeg = 45.0f;
    float nearPlane = 0.01f;
    float farPlane = 1000.0f;

private:
    QVector3D m_target;
    float m_yaw = 30.0f;
    float m_pitch = 30.0f;
    float m_distance = 10.0f;
    float m_aspect = 1.0f;
};
