#include "UavMarkerRenderer.h"
#include <QtMath>
#include <vector>

static const char* UAV_VS = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 uMVP;
void main() { gl_Position = uMVP * vec4(aPos, 1.0); }
)";

static const char* UAV_FS = R"(
#version 330 core
uniform vec4 uColor;
out vec4 FragColor;
void main() { FragColor = uColor; }
)";

UavMarkerRenderer::UavMarkerRenderer() {}

UavMarkerRenderer::~UavMarkerRenderer() {
    delete m_prog;
}

void UavMarkerRenderer::initialize() {
    QOpenGLExtraFunctions::initializeOpenGLFunctions();
    m_prog = new QOpenGLShaderProgram();
    m_prog->addShaderFromSourceCode(QOpenGLShader::Vertex,   UAV_VS);
    m_prog->addShaderFromSourceCode(QOpenGLShader::Fragment, UAV_FS);
    m_prog->link();
    m_initialized = true;
}

void UavMarkerRenderer::render(const QMatrix4x4& view, const QMatrix4x4& proj) {
    if (!m_initialized || !m_hasData) return;
    QMatrix4x4 mvp = proj * view;
    drawMarker(mvp);
    drawArrow(mvp);
}

void UavMarkerRenderer::drawMarker(const QMatrix4x4& mvp) {
    // 无人机位置：橙色大点 + 小十字
    std::vector<float> pt = {m_pos.x(), m_pos.y(), m_pos.z()};

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float), pt.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    m_prog->bind();
    m_prog->setUniformValue("uMVP",   mvp);
    m_prog->setUniformValue("uColor", QVector4D(1.0f, 0.55f, 0.0f, 1.0f));
    glPointSize(14.0f);
    glDrawArrays(GL_POINTS, 0, 1);
    m_prog->release();

    glBindVertexArray(0);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    // 垂直线（连接到地面）
    std::vector<float> vline = {
        m_pos.x(), 0.0f, m_pos.z(),
        m_pos.x(), m_pos.y(), m_pos.z()
    };
    GLuint vao2, vbo2;
    glGenVertexArrays(1, &vao2);
    glGenBuffers(1, &vbo2);
    glBindVertexArray(vao2);
    glBindBuffer(GL_ARRAY_BUFFER, vbo2);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vline.size() * sizeof(float)),
                 vline.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    m_prog->bind();
    m_prog->setUniformValue("uMVP",   mvp);
    m_prog->setUniformValue("uColor", QVector4D(1.0f, 0.55f, 0.0f, 0.5f));
    glLineWidth(1.0f);
    glDrawArrays(GL_LINES, 0, 2);
    m_prog->release();

    glBindVertexArray(0);
    glDeleteBuffers(1, &vbo2);
    glDeleteVertexArrays(1, &vao2);
}

void UavMarkerRenderer::drawArrow(const QMatrix4x4& mvp) {
    // 航向箭头：从无人机位置向 yaw 方向延伸（在 OpenGL xz 平面）
    // yaw=0 -> North -> OpenGL -Z; yaw=90 -> East -> OpenGL +X
    float yawRad = qDegreesToRadians(m_yaw);
    float len = 12.0f;
    float dx =  len * std::sin(yawRad);  // East -> +X
    float dz = -len * std::cos(yawRad);  // North -> -Z

    std::vector<float> arrow = {
        m_pos.x(), m_pos.y(), m_pos.z(),
        m_pos.x() + dx, m_pos.y(), m_pos.z() + dz
    };

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(arrow.size() * sizeof(float)),
                 arrow.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    m_prog->bind();
    m_prog->setUniformValue("uMVP",   mvp);
    m_prog->setUniformValue("uColor", QVector4D(1.0f, 0.9f, 0.0f, 1.0f));
    glLineWidth(2.5f);
    glDrawArrays(GL_LINES, 0, 2);
    m_prog->release();

    glBindVertexArray(0);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}
