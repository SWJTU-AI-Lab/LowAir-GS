#pragma once
#include "Effect.h"
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <vector>

// 光晕/脉冲特效渲染器（OpenGL 圆环 + 淡出动画）
class GlowEffectRenderer : public QOpenGLExtraFunctions {
public:
    GlowEffectRenderer() = default;
    ~GlowEffectRenderer();

    void initialize();
    void render(const QMatrix4x4& mvp, const std::vector<Effect>& effects);
    void cleanup();

private:
    void buildRingGeometry();

    QOpenGLShaderProgram m_prog;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    int    m_ringVertexCount = 0;
    bool   m_initialized = false;
};
