#pragma once
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include "../gaussian/GaussianCloud.h"

// Point Mode 渲染器：每个 Gaussian 显示为彩色点精灵
class GaussianPointRenderer : protected QOpenGLExtraFunctions {
public:
    GaussianPointRenderer() = default;
    ~GaussianPointRenderer();

    void initialize();
    void upload(const GaussianCloud &cloud);
    void render(const QMatrix4x4 &mvp, float pointSize, bool debugColor);
    void cleanup();

    bool isInitialized() const { return m_initialized; }
    int pointCount() const { return m_pointCount; }

private:
    bool m_initialized = false;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    int m_pointCount = 0;
    QOpenGLShaderProgram m_shader;
};
