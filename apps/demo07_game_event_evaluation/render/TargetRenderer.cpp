#include "TargetRenderer.h"
#include <cmath>
#include <vector>

static const char* TGT_VERT = R"(
#version 330 core
layout(location=0) in vec3 aPos;
uniform mat4 uMVP;
uniform vec3 uCenter;
uniform float uRadius;
void main(){
    gl_Position = uMVP * vec4(uCenter + aPos * uRadius, 1.0);
}
)";

static const char* TGT_FRAG = R"(
#version 330 core
uniform vec4 uColor;
out vec4 fragColor;
void main(){ fragColor = uColor; }
)";

TargetRenderer::~TargetRenderer() { cleanup(); }

void TargetRenderer::initialize() {
    initializeOpenGLFunctions();
    m_prog.addShaderFromSourceCode(QOpenGLShader::Vertex,   TGT_VERT);
    m_prog.addShaderFromSourceCode(QOpenGLShader::Fragment, TGT_FRAG);
    m_prog.link();
    buildSphereGeometry();
    m_initialized = true;
}

void TargetRenderer::buildSphereGeometry() {
    // 简化球：纬线圆 + 经线圆
    const int N = 32;
    std::vector<float> verts;
    // 赤道圆
    for (int i = 0; i <= N; ++i) {
        float a = 2.0f * 3.14159265f * i / N;
        verts.push_back(cosf(a)); verts.push_back(0.0f); verts.push_back(sinf(a));
    }
    // 子午圆
    for (int i = 0; i <= N; ++i) {
        float a = 2.0f * 3.14159265f * i / N;
        verts.push_back(cosf(a)); verts.push_back(sinf(a)); verts.push_back(0.0f);
    }
    m_vertexCount = (int)verts.size() / 3;

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void TargetRenderer::render(const QMatrix4x4& mvp, const std::vector<VirtualTarget>& targets) {
    if (!m_initialized) return;
    m_prog.bind();
    m_prog.setUniformValue("uMVP", mvp);
    glBindVertexArray(m_vao);

    for (const auto& t : targets) {
        m_prog.setUniformValue("uCenter", t.position);
        m_prog.setUniformValue("uRadius", t.radius);
        // 已完成：绿色；未完成：橙色
        QVector4D color = t.completed
            ? QVector4D(0.2f, 1.0f, 0.3f, 0.9f)
            : QVector4D(1.0f, 0.6f, 0.1f, 0.9f);
        m_prog.setUniformValue("uColor", color);
        int halfCount = m_vertexCount / 2;
        glDrawArrays(GL_LINE_STRIP, 0, halfCount);
        glDrawArrays(GL_LINE_STRIP, halfCount, halfCount);
    }

    glBindVertexArray(0);
    m_prog.release();
}

void TargetRenderer::cleanup() {
    if (m_vao) { glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
    if (m_vbo) { glDeleteBuffers(1, &m_vbo);      m_vbo = 0; }
}
