#pragma once
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>

class AxisRenderer : public QOpenGLExtraFunctions {
public:
    AxisRenderer() = default;
    ~AxisRenderer();
    void initialize(float length = 10.0f);
    void render(const QMatrix4x4& mvp);
    void cleanup();
private:
    QOpenGLShaderProgram m_prog;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    bool   m_initialized = false;
};
