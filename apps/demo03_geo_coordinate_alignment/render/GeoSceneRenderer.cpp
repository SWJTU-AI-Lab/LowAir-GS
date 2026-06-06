#include "GeoSceneRenderer.h"
#include <QtMath>
#include <cmath>
#include <QOpenGLContext>

// 简单颜色 shader
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

GeoSceneRenderer::GeoSceneRenderer() {}

GeoSceneRenderer::~GeoSceneRenderer() {
    delete m_prog;
}

void GeoSceneRenderer::initialize() {
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

void GeoSceneRenderer::buildGrid() {
    // 地面网格：在 OpenGL xz 平面（对应 ENU East-North 平面）
    // 网格范围：-200m 到 +200m，间距 25m
    m_gridVerts.clear();
    float step = 25.0f;
    float range = 200.0f;
    for (float x = -range; x <= range; x += step) {
        m_gridVerts.insert(m_gridVerts.end(), {x, 0.0f, -range});
        m_gridVerts.insert(m_gridVerts.end(), {x, 0.0f,  range});
    }
    for (float z = -range; z <= range; z += step) {
        m_gridVerts.insert(m_gridVerts.end(), {-range, 0.0f, z});
        m_gridVerts.insert(m_gridVerts.end(), { range, 0.0f, z});
    }
}

void GeoSceneRenderer::buildAxes() {
    // ENU 坐标轴（OpenGL 坐标）
    // East  (ENU E) -> OpenGL +X (红色)
    // North (ENU N) -> OpenGL -Z (蓝色)
    // Up    (ENU U) -> OpenGL +Y (黄色)
    float len = 50.0f;
    m_axisVerts = {
        // East: OpenGL +X
        0.0f, 0.0f, 0.0f,  len, 0.0f, 0.0f,
        // North: OpenGL -Z
        0.0f, 0.0f, 0.0f,  0.0f, 0.0f, -len,
        // Up: OpenGL +Y
        0.0f, 0.0f, 0.0f,  0.0f, len, 0.0f,
    };
}

void GeoSceneRenderer::buildOriginMarker() {
    // 原点标记：绿色十字
    float s = 5.0f;
    m_originVerts = {
        -s, 0.0f, 0.0f,  s, 0.0f, 0.0f,
        0.0f, 0.0f, -s,  0.0f, 0.0f, s,
        0.0f, -s, 0.0f,  0.0f, s, 0.0f,
    };
}

void GeoSceneRenderer::buildDistanceRings() {
    // 距离环：25m / 50m / 100m / 200m（在 ENU East-North 平面）
    m_ringVerts.clear();
    m_ringRadii = {25.0f, 50.0f, 100.0f, 200.0f};
    int segments = 64;
    for (float r : m_ringRadii) {
        std::vector<float> ring;
        for (int i = 0; i <= segments; ++i) {
            float angle = 2.0f * M_PI * i / segments;
            // ENU East -> OpenGL X, ENU North -> OpenGL -Z
            float x = r * std::cos(angle);
            float z = -r * std::sin(angle);
            ring.insert(ring.end(), {x, 0.0f, z});
        }
        m_ringVerts.push_back(ring);
    }
}

void GeoSceneRenderer::drawLines(const std::vector<float>& verts,
                                  const QMatrix4x4& mvp,
                                  float r, float g, float b, float a,
                                  float lineWidth) {
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

void GeoSceneRenderer::drawPoints(const std::vector<float>& verts,
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

void GeoSceneRenderer::render(const QMatrix4x4& view, const QMatrix4x4& proj) {
    if (!m_initialized) return;

    QMatrix4x4 mvp = proj * view;

    // 地面网格（深灰色）
    if (m_showGrid) {
        drawLines(m_gridVerts, mvp, 0.25f, 0.25f, 0.28f, 1.0f, 1.0f);
    }

    // ENU 坐标轴
    if (m_showAxis) {
        // East (红色)
        std::vector<float> eastAxis(m_axisVerts.begin(), m_axisVerts.begin() + 6);
        drawLines(eastAxis, mvp, 0.9f, 0.2f, 0.2f, 1.0f, 2.5f);
        // North (蓝色)
        std::vector<float> northAxis(m_axisVerts.begin() + 6, m_axisVerts.begin() + 12);
        drawLines(northAxis, mvp, 0.2f, 0.4f, 0.9f, 1.0f, 2.5f);
        // Up (黄色)
        std::vector<float> upAxis(m_axisVerts.begin() + 12, m_axisVerts.end());
        drawLines(upAxis, mvp, 0.9f, 0.85f, 0.1f, 1.0f, 2.5f);
    }

    // 原点标记（绿色）
    if (m_showOrigin) {
        drawLines(m_originVerts, mvp, 0.1f, 0.9f, 0.3f, 1.0f, 3.0f);
        // 原点大点
        std::vector<float> originPt = {0.0f, 0.0f, 0.0f};
        drawPoints(originPt, mvp, 0.1f, 0.9f, 0.3f, 12.0f);
    }

    // 距离环（灰色，不同透明度）
    if (m_showRings) {
        float alphas[] = {0.5f, 0.45f, 0.4f, 0.35f};
        for (size_t i = 0; i < m_ringVerts.size(); ++i) {
            // 将环顶点转为 LINE_STRIP 格式（首尾相连）
            const auto& ring = m_ringVerts[i];
            // 转为 LINES 格式
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
