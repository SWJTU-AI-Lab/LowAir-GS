#include "GaussianPointRenderer.h"
#include <QOpenGLContext>
#include <QDebug>

static const char *POINT_VERT = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 aColor;
uniform mat4 uMVP;
uniform float uPointSize;
uniform bool uDebugColor;
out vec4 vColor;
void main() {
    gl_Position = uMVP * vec4(aPos, 1.0);
    gl_PointSize = uPointSize;
    if (uDebugColor) {
        // Debug: 用位置哈希着色
        float h = fract(sin(dot(aPos, vec3(12.9898, 78.233, 45.164))) * 43758.5453);
        vColor = vec4(h, fract(h * 3.0), fract(h * 7.0), aColor.a);
    } else {
        vColor = aColor;
    }
}
)";

static const char *POINT_FRAG = R"(
#version 330 core
in vec4 vColor;
out vec4 fragColor;
void main() {
    // 圆形点精灵
    vec2 coord = gl_PointCoord - vec2(0.5);
    if (dot(coord, coord) > 0.25) discard;
    fragColor = vColor;
}
)";

GaussianPointRenderer::~GaussianPointRenderer() {
    cleanup();
}

void GaussianPointRenderer::initialize() {
    initializeOpenGLFunctions();
    m_shader.addShaderFromSourceCode(QOpenGLShader::Vertex, POINT_VERT);
    m_shader.addShaderFromSourceCode(QOpenGLShader::Fragment, POINT_FRAG);
    m_shader.link();

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    m_initialized = true;
}

void GaussianPointRenderer::upload(const GaussianCloud &cloud) {
    if (!m_initialized) return;

    // 每个顶点：position (3 float) + color (4 float) = 7 floats = 28 bytes
    const auto &pts = cloud.points();
    m_pointCount = pts.size();
    if (m_pointCount == 0) return;

    QVector<float> buf;
    buf.reserve(m_pointCount * 7);
    for (const auto &pt : pts) {
        buf.append(pt.position.x());
        buf.append(pt.position.y());
        buf.append(pt.position.z());
        buf.append(pt.color.x());
        buf.append(pt.color.y());
        buf.append(pt.color.z());
        buf.append(pt.opacity);
    }

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, buf.size() * sizeof(float), buf.constData(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);
}

void GaussianPointRenderer::render(const QMatrix4x4 &mvp, float pointSize, bool debugColor) {
    if (!m_initialized || m_pointCount == 0) return;

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_shader.bind();
    m_shader.setUniformValue("uMVP", mvp);
    m_shader.setUniformValue("uPointSize", pointSize);
    m_shader.setUniformValue("uDebugColor", debugColor);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_POINTS, 0, m_pointCount);
    glBindVertexArray(0);

    m_shader.release();
    glDisable(GL_BLEND);
    glDisable(GL_PROGRAM_POINT_SIZE);
}

void GaussianPointRenderer::cleanup() {
    if (m_vao) { glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
    if (m_vbo) { glDeleteBuffers(1, &m_vbo); m_vbo = 0; }
    m_initialized = false;
}
