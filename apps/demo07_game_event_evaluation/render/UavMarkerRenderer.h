#pragma once
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <QVector3D>

// 无人机标记渲染器（橙色方块 + 航向箭头）
class UavMarkerRenderer : public QOpenGLExtraFunctions {
public:
    UavMarkerRenderer() = default;
    ~UavMarkerRenderer();
    void initialize();
    void render(const QMatrix4x4& mvp, const QVector3D& pos, float yawDeg);
    void cleanup();
private:
    QOpenGLShaderProgram m_prog;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    bool   m_initialized = false;
};
