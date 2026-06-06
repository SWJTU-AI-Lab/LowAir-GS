#pragma once

// TrajectoryRenderer.h
// 历史轨迹渲染器：青色折线，最多保留 2000 个点

#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <QVector3D>
#include <deque>

class TrajectoryRenderer : protected QOpenGLExtraFunctions {
public:
    TrajectoryRenderer();
    ~TrajectoryRenderer();

    void initialize();
    void render(const QMatrix4x4& view, const QMatrix4x4& proj);

    void addPoint(const QVector3D& pt);
    void clear();
    int  pointCount() const { return static_cast<int>(m_points.size()); }

    static constexpr int MAX_POINTS = 2000;

private:
    QOpenGLShaderProgram* m_prog = nullptr;
    std::deque<QVector3D> m_points;
    bool m_initialized = false;
};
