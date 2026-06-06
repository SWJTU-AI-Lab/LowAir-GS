#include "GaussianCloud.h"
#include <limits>

void GaussianCloud::clear() {
    m_points.clear();
    m_bboxMin = QVector3D(0, 0, 0);
    m_bboxMax = QVector3D(0, 0, 0);
    hasOpacity = false;
    hasScale = false;
    hasRotation = false;
    hasSH = false;
    filePath.clear();
    formatType.clear();
    loadTimeMs = 0;
    fileSizeBytes = 0;
}

void GaussianCloud::addPoint(const GaussianPoint &pt) {
    m_points.append(pt);
}

void GaussianCloud::computeBoundingBox() {
    if (m_points.isEmpty()) {
        m_bboxMin = QVector3D(0, 0, 0);
        m_bboxMax = QVector3D(0, 0, 0);
        return;
    }
    float inf = std::numeric_limits<float>::max();
    m_bboxMin = QVector3D(inf, inf, inf);
    m_bboxMax = QVector3D(-inf, -inf, -inf);
    for (const auto &pt : m_points) {
        m_bboxMin.setX(qMin(m_bboxMin.x(), pt.position.x()));
        m_bboxMin.setY(qMin(m_bboxMin.y(), pt.position.y()));
        m_bboxMin.setZ(qMin(m_bboxMin.z(), pt.position.z()));
        m_bboxMax.setX(qMax(m_bboxMax.x(), pt.position.x()));
        m_bboxMax.setY(qMax(m_bboxMax.y(), pt.position.y()));
        m_bboxMax.setZ(qMax(m_bboxMax.z(), pt.position.z()));
    }
}
