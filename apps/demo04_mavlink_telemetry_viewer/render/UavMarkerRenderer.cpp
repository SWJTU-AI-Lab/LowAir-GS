#include "UavMarkerRenderer.h"
#include <cmath>

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

UavMarkerRenderer::UavMarkerRenderer() {}
UavMarkerRenderer::~UavMarkerRenderer() { delete m_prog; }

void UavMarkerRenderer::initialize() {
    QOpenGLExtraFunctions::initializeOpenGLFunctions();
    m_prog = new QOpenGLShaderProgram();
    m_prog->addShaderFromSourceCode(QOpenGLShader::Vertex,   VS_SRC);
    m_prog->addShaderFromSourceCode(QOpenGLShader::Fragment, FS_SRC);
    m_prog->link();
    m_initialized = true;
}

void UavMarkerRenderer::render(const QMatrix4x4& view, const QMatrix4x4& proj,
                                const QVector3D& pos, float yawDeg, bool connected, bool gpsValid) {
    if (!m_initialized) return;

    QMatrix4x4 model;
    model.translate(pos);
    QMatrix4x4 mvp = proj * view * model;

    // 无人机本体：橙色方块（8 条边线）
    float s = 3.0f;
    float r = connected ? 1.0f : 0.5f;
    float g = connected ? 0.55f : 0.55f;
    float b = connected ? 0.0f : 0.0f;

    std::vector<float> box = {
        -s,-s,-s,  s,-s,-s,   s,-s,-s,  s, s,-s,   s, s,-s, -s, s,-s,  -s, s,-s, -s,-s,-s,
        -s,-s, s,  s,-s, s,   s,-s, s,  s, s, s,   s, s, s, -s, s, s,  -s, s, s, -s,-s, s,
        -s,-s,-s, -s,-s, s,   s,-s,-s,  s,-s, s,   s, s,-s,  s, s, s,  -s, s,-s, -s, s, s,
    };
    drawLines(box, mvp, r, g, b, 1.0f, 2.0f);

    // 中心点
    drawPoints({0.0f, 0.0f, 0.0f}, mvp, r, g, b, 16.0f);

    // 航向箭头（青色，在水平面内）
    float rad = yawDeg * static_cast<float>(M_PI) / 180.0f;
    float arrowLen = 10.0f;
    // OpenGL: East=+X, North=-Z, 航向从北顺时针
    float ax = arrowLen *  std::sin(rad);
    float az = arrowLen * -std::cos(rad);
    std::vector<float> arrow = {
        0.0f, 0.0f, 0.0f,  ax, 0.0f, az,
        ax, 0.0f, az,  ax - 3.0f * std::cos(rad), 0.0f, az - 3.0f * std::sin(rad),
        ax, 0.0f, az,  ax + 3.0f * std::cos(rad), 0.0f, az + 3.0f * std::sin(rad),
    };
    drawLines(arrow, mvp, 0.0f, 0.9f, 0.9f, 1.0f, 2.5f);

    // GPS 状态指示（红/绿小点）
    float gr = gpsValid ? 0.1f : 0.9f;
    float gg = gpsValid ? 0.9f : 0.2f;
    float gb = gpsValid ? 0.1f : 0.2f;
    std::vector<float> gpsDot = {0.0f, s + 2.0f, 0.0f};
    drawPoints(gpsDot, mvp, gr, gg, gb, 10.0f);
}

void UavMarkerRenderer::drawLines(const std::vector<float>& verts, const QMatrix4x4& mvp,
                                   float r, float g, float b, float a, float lineWidth) {
    if (verts.empty()) return;
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
    m_prog->setUniformValue("uColor", QVector4D(r, g, b, a));
    glLineWidth(lineWidth);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(verts.size() / 3));
    m_prog->release();
    glBindVertexArray(0);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

void UavMarkerRenderer::drawPoints(const std::vector<float>& verts, const QMatrix4x4& mvp,
                                    float r, float g, float b, float pointSize) {
    if (verts.empty()) return;
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
    m_prog->setUniformValue("uColor", QVector4D(r, g, b, 1.0f));
    glPointSize(pointSize);
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(verts.size() / 3));
    m_prog->release();
    glBindVertexArray(0);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}
