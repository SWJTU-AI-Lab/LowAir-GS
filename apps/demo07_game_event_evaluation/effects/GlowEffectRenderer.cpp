#include "GlowEffectRenderer.h"
#include <cmath>
#include <vector>

static const char* GLOW_VERT = R"(
#version 330 core
layout(location=0) in vec3 aPos;
uniform mat4 uMVP;
uniform vec3 uCenter;
uniform float uRadius;
void main(){
    vec3 p = uCenter + aPos * uRadius;
    gl_Position = uMVP * vec4(p, 1.0);
}
)";

static const char* GLOW_FRAG = R"(
#version 330 core
uniform vec4 uColor;
out vec4 fragColor;
void main(){
    fragColor = uColor;
}
)";

GlowEffectRenderer::~GlowEffectRenderer() { cleanup(); }

void GlowEffectRenderer::initialize() {
    initializeOpenGLFunctions();
    m_prog.addShaderFromSourceCode(QOpenGLShader::Vertex,   GLOW_VERT);
    m_prog.addShaderFromSourceCode(QOpenGLShader::Fragment, GLOW_FRAG);
    m_prog.link();
    buildRingGeometry();
    m_initialized = true;
}

void GlowEffectRenderer::buildRingGeometry() {
    const int N = 64;
    std::vector<float> verts;
    for (int i = 0; i <= N; ++i) {
        float a = 2.0f * 3.14159265f * i / N;
        verts.push_back(cosf(a));
        verts.push_back(0.0f);
        verts.push_back(sinf(a));
    }
    m_ringVertexCount = (int)verts.size() / 3;

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void GlowEffectRenderer::render(const QMatrix4x4& mvp, const std::vector<Effect>& effects) {
    if (!m_initialized) return;
    m_prog.bind();
    m_prog.setUniformValue("uMVP", mvp);
    glBindVertexArray(m_vao);

    for (const auto& e : effects) {
        if (!e.active) continue;
        float a = e.alpha();
        if (a <= 0.0f) continue;

        QVector4D color = e.color;
        color.setW(a);
        m_prog.setUniformValue("uCenter", e.position);
        m_prog.setUniformValue("uRadius", e.radius);
        m_prog.setUniformValue("uColor",  color);

        // 绘制多个同心环（Glow 效果）
        int rings = (e.type == EffectType::Pulse) ? 3 : 1;
        for (int r = 0; r < rings; ++r) {
            m_prog.setUniformValue("uRadius", e.radius * (1.0f + r * 0.3f));
            glDrawArrays(GL_LINE_STRIP, 0, m_ringVertexCount);
        }
    }

    glBindVertexArray(0);
    m_prog.release();
}

void GlowEffectRenderer::cleanup() {
    if (m_vao) { glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
    if (m_vbo) { glDeleteBuffers(1, &m_vbo);      m_vbo = 0; }
}
