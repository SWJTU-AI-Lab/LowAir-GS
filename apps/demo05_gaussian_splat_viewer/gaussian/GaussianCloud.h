#pragma once
#include <QVector>
#include <QVector3D>
#include <QString>
#include "GaussianPoint.h"

// Gaussian 点云容器，管理加载后的全部 Gaussian 数据
class GaussianCloud {
public:
    GaussianCloud() = default;

    void clear();
    void addPoint(const GaussianPoint &pt);
    int count() const { return m_points.size(); }
    const QVector<GaussianPoint>& points() const { return m_points; }
    QVector<GaussianPoint>& points() { return m_points; }

    // 包围盒
    QVector3D bboxMin() const { return m_bboxMin; }
    QVector3D bboxMax() const { return m_bboxMax; }
    QVector3D bboxSize() const { return m_bboxMax - m_bboxMin; }
    QVector3D center() const { return (m_bboxMin + m_bboxMax) * 0.5f; }

    void computeBoundingBox();

    // 元数据
    QString filePath;
    QString formatType;
    bool hasOpacity = false;
    bool hasScale = false;
    bool hasRotation = false;
    bool hasSH = false;
    qint64 loadTimeMs = 0;
    qint64 fileSizeBytes = 0;

private:
    QVector<GaussianPoint> m_points;
    QVector3D m_bboxMin;
    QVector3D m_bboxMax;
};
