#include "GridRenderer.h"
#include <vector>

static const char* GRID_VERT = R"(
#version 330 core
layout(location=0) in vec3 aPos;
uniform mat4 uMVP;
void main(){ gl_Position = uMVP * vec4(aPos, 1.0); }
)";
static const char* GRID_FRAG = R"(
#version 330 core
out vec4 fragColor;
void main(){ fragColor = vec4(0.35, 0.35, 0.45, 1.0); }
)";

GridRenderer::~GridRenderer() { cleanup(); }

void GridRenderer::initialize(float size, int divisions) {
    initializeOpenGLFunctions();
    m_prog.addShaderFromSourceCode(QOpenGLShader::Vertex,   GRID_VERT);
    m_prog.addShaderFromSourceCode(QOpenGLShader::Fragment, GRID_FRAG);
    m_prog.link();

    std::vector<float> verts;
    float step = size * 2.0f / divisions;
    for (int i = 0; i <= divisions; ++i) {
        float v = -size + i * step;
        verts.insert(verts.end(), {-size, 0.0f, v,  size, 0.0f, v});
        verts.insert(verts.end(), {v, 0.0f, -size,  v, 0.0f, size});
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
    m_initialized = true;
}

void GridRenderer::render(const QMatrix4x4& mvp) {
    if (!m_initialized) return;
    m_prog.bind();
    m_prog.setUniformValue("uMVP", mvp);
    glBindVertexArray(m_vao);
    glDrawArrays(GL_LINES, 0, m_vertexCount);
    glBindVertexArray(0);
    m_prog.release();
}

void GridRenderer::cleanup() {
    if (m_vao) { glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
    if (m_vbo) { glDeleteBuffers(1, &m_vbo);      m_vbo = 0; }
}
