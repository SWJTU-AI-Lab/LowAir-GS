#include "TelemetrySceneRenderer.h"
#include <QtMath>
#include <cmath>
#include <QOpenGLContext>

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

TelemetrySceneRenderer::TelemetrySceneRenderer() {}

TelemetrySceneRenderer::~TelemetrySceneRenderer() {
    delete m_prog;
}

void TelemetrySceneRenderer::initialize() {
    QOpenGLExtraFunctions::initializeOpenGLFunctions();
    m_prog = new QOpenGLShaderProgram();
    m_prog->addShaderFromSourceCode(QOpenGLShader::Vertex,   VS_SRC);
    m_prog->addShaderFromSourceCode(QOpenGLShader::Fragment, FS_SRC);
    m_prog->link();
    buildGrid();
    buildAxes();
    buildOriginMarker();
    buildDistanceRings();
    m_initialized = true;
}

void TelemetrySceneRenderer::buildGrid() {
    m_gridVerts.clear();
    float step = 25.0f, range = 300.0f;
    for (float x = -range; x <= range; x += step) {
        m_gridVerts.insert(m_gridVerts.end(), {x, 0.0f, -range});
        m_gridVerts.insert(m_gridVerts.end(), {x, 0.0f,  range});
    }
    for (float z = -range; z <= range; z += step) {
        m_gridVerts.insert(m_gridVerts.end(), {-range, 0.0f, z});
        m_gridVerts.insert(m_gridVerts.end(), { range, 0.0f, z});
    }
}

void TelemetrySceneRenderer::buildAxes() {
    float len = 60.0f;
    m_axisVerts = {
        0.0f, 0.0f, 0.0f,  len,  0.0f, 0.0f,   // East  -> OpenGL +X (红)
        0.0f, 0.0f, 0.0f,  0.0f, 0.0f, -len,   // North -> OpenGL -Z (蓝)
        0.0f, 0.0f, 0.0f,  0.0f, len,  0.0f,   // Up    -> OpenGL +Y (黄)
    };
}

void TelemetrySceneRenderer::buildOriginMarker() {
    float s = 6.0f;
    m_originVerts = {
        -s, 0.0f, 0.0f,  s, 0.0f, 0.0f,
        0.0f, 0.0f, -s,  0.0f, 0.0f, s,
        0.0f, -s, 0.0f,  0.0f, s, 0.0f,
    };
}

void TelemetrySceneRenderer::buildDistanceRings() {
    m_ringVerts.clear();
    m_ringRadii = {50.0f, 100.0f, 200.0f, 300.0f};
    int segs = 72;
    for (float r : m_ringRadii) {
        std::vector<float> ring;
        for (int i = 0; i <= segs; ++i) {
            float angle = 2.0f * static_cast<float>(M_PI) * i / segs;
            ring.insert(ring.end(), {r * std::cos(angle), 0.0f, -r * std::sin(angle)});
        }
        m_ringVerts.push_back(ring);
    }
}

void TelemetrySceneRenderer::drawLines(const std::vector<float>& verts,
                                        const QMatrix4x4& mvp,
                                        float r, float g, float b, float a, float lineWidth) {
    if (verts.empty()) return;
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

void TelemetrySceneRenderer::drawPoints(const std::vector<float>& verts,
                                         const QMatrix4x4& mvp,
                                         float r, float g, float b, float pointSize) {
    if (verts.empty()) return;
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

void TelemetrySceneRenderer::render(const QMatrix4x4& view, const QMatrix4x4& proj) {
    if (!m_initialized) return;
    QMatrix4x4 mvp = proj * view;

    if (m_showGrid)
        drawLines(m_gridVerts, mvp, 0.22f, 0.22f, 0.25f, 1.0f, 1.0f);

    if (m_showAxis) {
        std::vector<float> e(m_axisVerts.begin(),      m_axisVerts.begin() + 6);
        std::vector<float> n(m_axisVerts.begin() + 6,  m_axisVerts.begin() + 12);
        std::vector<float> u(m_axisVerts.begin() + 12, m_axisVerts.end());
        drawLines(e, mvp, 0.9f, 0.2f, 0.2f, 1.0f, 2.5f);  // East  红
        drawLines(n, mvp, 0.2f, 0.4f, 0.9f, 1.0f, 2.5f);  // North 蓝
        drawLines(u, mvp, 0.9f, 0.85f,0.1f, 1.0f, 2.5f);  // Up    黄
    }

    if (m_showOrigin) {
        drawLines(m_originVerts, mvp, 0.1f, 0.9f, 0.3f, 1.0f, 3.0f);
        drawPoints({0.0f, 0.0f, 0.0f}, mvp, 0.1f, 0.9f, 0.3f, 14.0f);
    }

    if (m_showRings) {
        float alphas[] = {0.5f, 0.45f, 0.4f, 0.35f};
        for (size_t i = 0; i < m_ringVerts.size(); ++i) {
            const auto& ring = m_ringVerts[i];
            std::vector<float> lineVerts;
            for (size_t j = 0; j + 3 < ring.size(); j += 3) {
                lineVerts.insert(lineVerts.end(), ring.begin() + j, ring.begin() + j + 3);
                lineVerts.insert(lineVerts.end(), ring.begin() + j + 3, ring.begin() + j + 6);
            }
            float alpha = (i < 4) ? alphas[i] : 0.3f;
            drawLines(lineVerts, mvp, 0.5f, 0.5f, 0.55f, alpha, 1.0f);
        }
    }
}
