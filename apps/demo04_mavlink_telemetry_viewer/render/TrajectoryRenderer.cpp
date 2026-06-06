#include "TrajectoryRenderer.h"

static const char* VS_SRC = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 uMVP;
void main() { gl_Position = uMVP * vec4(aPos, 1.0); }
)";
static const char* FS_SRC = R"(
#version 330 core
uniform vec4 uColor;
out vec4 FragColor;
void main() { FragColor = uColor; }
)";

TrajectoryRenderer::TrajectoryRenderer() {}
TrajectoryRenderer::~TrajectoryRenderer() { delete m_prog; }

void TrajectoryRenderer::initialize() {
    QOpenGLExtraFunctions::initializeOpenGLFunctions();
    m_prog = new QOpenGLShaderProgram();
    m_prog->addShaderFromSourceCode(QOpenGLShader::Vertex,   VS_SRC);
    m_prog->addShaderFromSourceCode(QOpenGLShader::Fragment, FS_SRC);
    m_prog->link();
    m_initialized = true;
}

void TrajectoryRenderer::addPoint(const QVector3D& pt) {
    if (static_cast<int>(m_points.size()) >= MAX_POINTS) {
        m_points.erase(m_points.begin());
    }
    m_points.push_back(pt);
}

void TrajectoryRenderer::clear() {
    m_points.clear();
}

void TrajectoryRenderer::render(const QMatrix4x4& view, const QMatrix4x4& proj) {
    if (!m_initialized || m_points.size() < 2) return;

    // 构建 GL_LINES 顶点数组
    std::vector<float> verts;
    verts.reserve((m_points.size() - 1) * 6);
    for (size_t i = 0; i + 1 < m_points.size(); ++i) {
        verts.insert(verts.end(), {m_points[i].x(),   m_points[i].y(),   m_points[i].z()});
        verts.insert(verts.end(), {m_points[i+1].x(), m_points[i+1].y(), m_points[i+1].z()});
    }

    QMatrix4x4 mvp = proj * view;
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(verts.size() * sizeof(float)),
                 verts.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    m_prog->bind();
    m_prog->setUniformValue("uMVP",   mvp);
    m_prog->setUniformValue("uColor", QVector4D(0.0f, 0.9f, 0.9f, 0.85f));
    glLineWidth(1.8f);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(verts.size() / 3));
    m_prog->release();
    glBindVertexArray(0);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}
