#include "OrbitCamera.h"
#include <QtMath>
#include <algorithm>

OrbitCamera::OrbitCamera() {}

QMatrix4x4 OrbitCamera::viewMatrix() const {
    // 球坐标 -> 相机位置
    float azRad  = qDegreesToRadians(m_azimuth);
    float elRad  = qDegreesToRadians(m_elevation);

    QVector3D eye;
    eye.setX(m_distance * std::cos(elRad) * std::sin(azRad));
    eye.setY(m_distance * std::sin(elRad));
    eye.setZ(m_distance * std::cos(elRad) * std::cos(azRad));
    eye += m_target;

    QMatrix4x4 view;
    view.lookAt(eye, m_target, QVector3D(0, 1, 0));
    return view;
}

QMatrix4x4 OrbitCamera::projectionMatrix(float aspect) const {
    QMatrix4x4 proj;
    proj.perspective(m_fov, aspect, m_zNear, m_zFar);
    return proj;
}

void OrbitCamera::mousePressEvent(Qt::MouseButton button, const QPoint& pos) {
    m_pressedButton = button;
    m_lastPos = pos;
}

void OrbitCamera::mouseMoveEvent(const QPoint& pos) {
    QPoint delta = pos - m_lastPos;
    m_lastPos = pos;

    if (m_pressedButton == Qt::LeftButton) {
        // 旋转
        m_azimuth   += delta.x() * 0.5f;
        m_elevation -= delta.y() * 0.5f;
        m_elevation  = std::clamp(m_elevation, -89.0f, 89.0f);
    } else if (m_pressedButton == Qt::RightButton) {
        // 平移
        float scale = m_distance * 0.001f;
        m_target.setX(m_target.x() - delta.x() * scale);
        m_target.setY(m_target.y() + delta.y() * scale);
    }
}

void OrbitCamera::mouseReleaseEvent() {
    m_pressedButton = Qt::NoButton;
}

void OrbitCamera::wheelEvent(float delta) {
    m_distance *= (1.0f - delta * 0.001f);
    m_distance = std::clamp(m_distance, 1.0f, 2000.0f);
}

void OrbitCamera::reset() {
    m_azimuth   = 30.0f;
    m_elevation = 40.0f;
    m_distance  = 150.0f;
    m_target    = {0, 0, 0};
}

void OrbitCamera::fitToRadius(float radius) {
    m_distance = radius * 2.5f;
    m_target   = {0, 0, 0};
}
