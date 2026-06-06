#include "OrbitCamera.h"
#include <QtMath>

OrbitCamera::OrbitCamera() {
    m_target = QVector3D(0, 0, 0);
}

void OrbitCamera::setTarget(const QVector3D &target) { m_target = target; }
void OrbitCamera::setDistance(float dist) { m_distance = qMax(0.1f, dist); }
void OrbitCamera::setAspect(float aspect) { m_aspect = aspect; }

void OrbitCamera::orbit(float dYaw, float dPitch) {
    m_yaw += dYaw;
    m_pitch = qBound(-89.0f, m_pitch + dPitch, 89.0f);
}

void OrbitCamera::zoom(float delta) {
    m_distance = qMax(0.1f, m_distance - delta * m_distance * 0.1f);
}

void OrbitCamera::pan(float dx, float dy) {
    QMatrix4x4 v = viewMatrix();
    QVector3D right(v(0,0), v(0,1), v(0,2));
    QVector3D up(v(1,0), v(1,1), v(1,2));
    float scale = m_distance * 0.001f;
    m_target -= right * dx * scale;
    m_target += up * dy * scale;
}

void OrbitCamera::reset() {
    m_target = QVector3D(0, 0, 0);
    m_yaw = 30.0f;
    m_pitch = 30.0f;
    m_distance = 10.0f;
}

QVector3D OrbitCamera::position() const {
    float yawRad = qDegreesToRadians(m_yaw);
    float pitchRad = qDegreesToRadians(m_pitch);
    float x = m_distance * qCos(pitchRad) * qSin(yawRad);
    float y = m_distance * qSin(pitchRad);
    float z = m_distance * qCos(pitchRad) * qCos(yawRad);
    return m_target + QVector3D(x, y, z);
}

QMatrix4x4 OrbitCamera::viewMatrix() const {
    QMatrix4x4 m;
    m.lookAt(position(), m_target, QVector3D(0, 1, 0));
    return m;
}

QMatrix4x4 OrbitCamera::projMatrix() const {
    QMatrix4x4 m;
    m.perspective(fovDeg, m_aspect, nearPlane, farPlane);
    return m;
}

QMatrix4x4 OrbitCamera::mvpMatrix() const {
    return projMatrix() * viewMatrix();
}
