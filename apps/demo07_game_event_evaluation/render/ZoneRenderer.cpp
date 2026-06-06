#include "ZoneRenderer.h"

static const char* ZONE_VERT = R"(
#version 330 core
layout(location=0) in vec3 aPos;
uniform mat4 uMVP;
uniform vec3 uCenter;
uniform vec3 uSize;
void main(){
    gl_Position = uMVP * vec4(uCenter + aPos * uSize * 0.5, 1.0);
}
)";

static const char* ZONE_FRAG = R"(
#version 330 core
uniform vec4 uColor;
out vec4 fragColor;
void main(){ fragColor = uColor; }
)";

ZoneRenderer::~ZoneRenderer() { cleanup(); }

void ZoneRenderer::initialize() {
    initializeOpenGLFunctions();
    m_prog.addShaderFromSourceCode(QOpenGLShader::Vertex,   ZONE_VERT);
    m_prog.addShaderFromSourceCode(QOpenGLShader::Fragment, ZONE_FRAG);
    m_prog.link();
    buildBoxGeometry();
    m_initialized = true;
}

void ZoneRenderer::buildBoxGeometry() {
    // 单位立方体顶点（-1 到 1）
    static const float verts[] = {
        -1,-1,-1,  1,-1,-1,  1, 1,-1, -1, 1,-1,
        -1,-1, 1,  1,-1, 1,  1, 1, 1, -1, 1, 1
    };
    // 12 条棱
    static const unsigned short idx[] = {
        0,1, 1,2, 2,3, 3,0,
        4,5, 5,6, 6,7, 7,4,
        0,4, 1,5, 2,6, 3,7
    };
    m_indexCount = 24;

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ibo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
    glBindVertexArray(0);
}

void ZoneRenderer::render(const QMatrix4x4& mvp, const std::vector<TriggerZone>& zones) {
    if (!m_initialized) return;
    m_prog.bind();
    m_prog.setUniformValue("uMVP", mvp);
    glBindVertexArray(m_vao);

    for (const auto& z : zones) {
        m_prog.setUniformValue("uCenter", z.center);
        m_prog.setUniformValue("uSize",   z.size);
        QVector4D color = z.completed
            ? QVector4D(0.2f, 0.8f, 0.2f, 0.5f)
            : QVector4D(0.2f, 0.6f, 1.0f, 0.6f);
        m_prog.setUniformValue("uColor", color);
        glDrawElements(GL_LINES, m_indexCount, GL_UNSIGNED_SHORT, nullptr);
    }

    glBindVertexArray(0);
    m_prog.release();
}

void ZoneRenderer::cleanup() {
    if (m_vao) { glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
    if (m_vbo) { glDeleteBuffers(1, &m_vbo);      m_vbo = 0; }
    if (m_ibo) { glDeleteBuffers(1, &m_ibo);      m_ibo = 0; }
}
