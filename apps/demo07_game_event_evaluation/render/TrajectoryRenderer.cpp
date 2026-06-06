#include "TrajectoryRenderer.h"

static const char* TRAJ_VERT = R"(
#version 330 core
layout(location=0) in vec3 aPos;
uniform mat4 uMVP;
void main(){ gl_Position = uMVP * vec4(aPos, 1.0); }
)";

static const char* TRAJ_FRAG = R"(
#version 330 core
out vec4 fragColor;
void main(){ fragColor = vec4(0.0, 0.9, 0.9, 0.85); }
)";

TrajectoryRenderer::~TrajectoryRenderer() { cleanup(); }

void TrajectoryRenderer::initialize() {
    initializeOpenGLFunctions();
    m_prog.addShaderFromSourceCode(QOpenGLShader::Vertex,   TRAJ_VERT);
    m_prog.addShaderFromSourceCode(QOpenGLShader::Fragment, TRAJ_FRAG);
    m_prog.link();

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_POINTS * 3 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    m_initialized = true;
}

void TrajectoryRenderer::addPoint(const QVector3D& p) {
    if ((int)m_points.size() >= MAX_POINTS)
        m_points.erase(m_points.begin());
    m_points.push_back(p);
    m_dirty = true;
}

void TrajectoryRenderer::render(const QMatrix4x4& mvp) {
    if (!m_initialized || m_points.size() < 2) return;

    if (m_dirty) {
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
                        m_points.size() * 3 * sizeof(float),
                        m_points.data());
        m_dirty = false;
    }

    m_prog.bind();
    m_prog.setUniformValue("uMVP", mvp);
    glBindVertexArray(m_vao);
    glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)m_points.size());
    glBindVertexArray(0);
    m_prog.release();
}

void TrajectoryRenderer::clear() { m_points.clear(); m_dirty = true; }

void TrajectoryRenderer::cleanup() {
    if (m_vao) { glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
    if (m_vbo) { glDeleteBuffers(1, &m_vbo);      m_vbo = 0; }
}
