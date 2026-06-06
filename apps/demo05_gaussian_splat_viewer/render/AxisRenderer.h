#pragma once
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>

class AxisRenderer : protected QOpenGLExtraFunctions {
public:
    void initialize();
    void render(const QMatrix4x4 &mvp, float length = 2.0f);
    void cleanup();
private:
    bool m_initialized = false;
    GLuint m_vao = 0, m_vbo = 0;
    QOpenGLShaderProgram m_shader;
};
