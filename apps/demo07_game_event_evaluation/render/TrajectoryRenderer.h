#pragma once
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <QVector3D>
#include <vector>

// 轨迹渲染器（历史轨迹线，青色，最多 3000 点）
class TrajectoryRenderer : public QOpenGLExtraFunctions {
public:
    static constexpr int MAX_POINTS = 3000;

    TrajectoryRenderer() = default;
    ~TrajectoryRenderer();

    void initialize();
    void addPoint(const QVector3D& p);
    void render(const QMatrix4x4& mvp);
    void clear();
    void cleanup();
    int  pointCount() const { return (int)m_points.size(); }

private:
    QOpenGLShaderProgram m_prog;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    std::vector<QVector3D> m_points;
    bool m_initialized = false;
    bool m_dirty = false;
};
