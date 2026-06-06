#include "GridRenderer.h"

static const char *GRID_VERT = R"(
#version 330 core
layout(location=0) in vec3 aPos;
uniform mat4 uMVP;
void main() { gl_Position = uMVP * vec4(aPos, 1.0); }
)";
static const char *GRID_FRAG = R"(
#version 330 core
out vec4 fragColor;
void main() { fragColor = vec4(0.35, 0.35, 0.40, 1.0); }
)";

void GridRenderer::initialize() {
    initializeOpenGLFunctions();
    m_shader.addShaderFromSourceCode(QOpenGLShader::Vertex, GRID_VERT);
    m_shader.addShaderFromSourceCode(QOpenGLShader::Fragment, GRID_FRAG);
    m_shader.link();
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    m_initialized = true;
}

void GridRenderer::buildGrid(float gridSize, int divisions) {
    QVector<float> verts;
    float step = gridSize * 2.0f / divisions;
    float start = -gridSize;
    for (int i = 0; i <= divisions; ++i) {
        float v = start + i * step;
        verts << v << 0 << -gridSize << v << 0 << gridSize;
        verts << -gridSize << 0 << v << gridSize << 0 << v;
    }
    m_lineCount = verts.size() / 3;
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.constData(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindVertexArray(0);
    m_lastGridSize = gridSize;
    m_lastDivisions = divisions;
}

void GridRenderer::render(const QMatrix4x4 &mvp, float gridSize, int divisions) {
    if (!m_initialized) return;
    if (gridSize != m_lastGridSize || divisions != m_lastDivisions)
        buildGrid(gridSize, divisions);
    m_shader.bind();
    m_shader.setUniformValue("uMVP", mvp);
    glBindVertexArray(m_vao);
    glDrawArrays(GL_LINES, 0, m_lineCount);
    glBindVertexArray(0);
    m_shader.release();
}

void GridRenderer::cleanup() {
    if (m_vao) { glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
    if (m_vbo) { glDeleteBuffers(1, &m_vbo); m_vbo = 0; }
}
