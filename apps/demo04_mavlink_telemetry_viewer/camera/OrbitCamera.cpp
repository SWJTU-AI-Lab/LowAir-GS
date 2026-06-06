#include "OrbitCamera.h"
#include <QtMath>
#include <algorithm>

OrbitCamera::OrbitCamera() {}

QMatrix4x4 OrbitCamera::viewMatrix() const {
    float azRad = qDegreesToRadians(m_azimuth);
    float elRad = qDegreesToRadians(m_elevation);

    float x = m_target.x() + m_distance * qCos(elRad) * qSin(azRad);
    float y = m_target.y() + m_distance * qSin(elRad);
    float z = m_target.z() + m_distance * qCos(elRad) * qCos(azRad);

    QMatrix4x4 view;
    view.lookAt(QVector3D(x, y, z), m_target, QVector3D(0.0f, 1.0f, 0.0f));
    return view;
}

void OrbitCamera::mouseRotate(float dx, float dy) {
    m_azimuth   += dx * 0.4f;
    m_elevation += dy * 0.4f;
    m_elevation  = std::clamp(m_elevation, -89.0f, 89.0f);
}

void OrbitCamera::mousePan(float dx, float dy) {
    float scale = m_distance * 0.001f;
    m_target += QVector3D(-dx * scale, dy * scale, 0.0f);
}

void OrbitCamera::mouseZoom(float delta) {
    m_distance *= (1.0f - delta * 0.001f);
    m_distance  = std::clamp(m_distance, 1.0f, 3000.0f);
}

void OrbitCamera::reset() {
    m_azimuth   = 30.0f;
    m_elevation = 40.0f;
    m_distance  = 200.0f;
    m_target    = {0.0f, 0.0f, 0.0f};
}

void OrbitCamera::fitToRadius(float radius) {
    m_distance = radius * 2.5f;
}
