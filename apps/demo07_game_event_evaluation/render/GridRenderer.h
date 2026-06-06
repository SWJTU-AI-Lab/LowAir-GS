#pragma once
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>

// 地面网格渲染器
class GridRenderer : public QOpenGLExtraFunctions {
public:
    GridRenderer() = default;
    ~GridRenderer();
    void initialize(float size = 50.0f, int divisions = 20);
    void render(const QMatrix4x4& mvp);
    void cleanup();
private:
    QOpenGLShaderProgram m_prog;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    int    m_vertexCount = 0;
    bool   m_initialized = false;
};
