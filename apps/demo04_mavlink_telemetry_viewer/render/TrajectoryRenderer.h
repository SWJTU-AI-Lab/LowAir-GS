#pragma once
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <QVector3D>
#include <vector>

// 历史轨迹渲染器（青色折线，最多 3000 点）
class TrajectoryRenderer : public QOpenGLExtraFunctions {
public:
    TrajectoryRenderer();
    ~TrajectoryRenderer();

    void initialize();
    void addPoint(const QVector3D& pt);
    void clear();
    void render(const QMatrix4x4& view, const QMatrix4x4& proj);
    int  pointCount() const { return static_cast<int>(m_points.size()); }

private:
    QOpenGLShaderProgram* m_prog        = nullptr;
    bool                  m_initialized = false;
    std::vector<QVector3D> m_points;
    static constexpr int   MAX_POINTS = 3000;
};
