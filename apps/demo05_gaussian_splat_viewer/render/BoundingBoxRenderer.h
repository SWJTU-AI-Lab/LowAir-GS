#pragma once
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <QVector3D>

class BoundingBoxRenderer : protected QOpenGLExtraFunctions {
public:
    void initialize();
    void setBox(const QVector3D &bmin, const QVector3D &bmax);
    void render(const QMatrix4x4 &mvp);
    void cleanup();
private:
    bool m_initialized = false;
    GLuint m_vao = 0, m_vbo = 0, m_ebo = 0;
    QOpenGLShaderProgram m_shader;
};
