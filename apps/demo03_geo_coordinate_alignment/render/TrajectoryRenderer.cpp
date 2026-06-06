#include "TrajectoryRenderer.h"
#include <vector>

static const char* TRAJ_VS = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 uMVP;
void main() { gl_Position = uMVP * vec4(aPos, 1.0); }
)";

static const char* TRAJ_FS = R"(
#version 330 core
uniform vec4 uColor;
out vec4 FragColor;
void main() { FragColor = uColor; }
)";

TrajectoryRenderer::TrajectoryRenderer() {}

TrajectoryRenderer::~TrajectoryRenderer() {
    delete m_prog;
}

void TrajectoryRenderer::initialize() {
    QOpenGLExtraFunctions::initializeOpenGLFunctions();
    m_prog = new QOpenGLShaderProgram();
    m_prog->addShaderFromSourceCode(QOpenGLShader::Vertex,   TRAJ_VS);
    m_prog->addShaderFromSourceCode(QOpenGLShader::Fragment, TRAJ_FS);
    m_prog->link();
    m_initialized = true;
}

void TrajectoryRenderer::addPoint(const QVector3D& pt) {
    m_points.push_back(pt);
    if (static_cast<int>(m_points.size()) > MAX_POINTS) {
        m_points.pop_front();
    }
}

void TrajectoryRenderer::clear() {
    m_points.clear();
}

void TrajectoryRenderer::render(const QMatrix4x4& view, const QMatrix4x4& proj) {
    if (!m_initialized || m_points.size() < 2) return;

    // 构建 LINES 顶点数组
    std::vector<float> verts;
    verts.reserve(m_points.size() * 6);
    for (size_t i = 0; i + 1 < m_points.size(); ++i) {
        verts.push_back(m_points[i].x());
        verts.push_back(m_points[i].y());
        verts.push_back(m_points[i].z());
        verts.push_back(m_points[i+1].x());
        verts.push_back(m_points[i+1].y());
        verts.push_back(m_points[i+1].z());
    }

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(verts.size() * sizeof(float)),
                 verts.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    QMatrix4x4 mvp = proj * view;
    m_prog->bind();
    m_prog->setUniformValue("uMVP",   mvp);
    m_prog->setUniformValue("uColor", QVector4D(0.0f, 0.85f, 0.85f, 0.85f));
    glLineWidth(1.5f);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(verts.size() / 3));
    m_prog->release();

    glBindVertexArray(0);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}
