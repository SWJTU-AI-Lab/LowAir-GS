#include "GaussianBillboardRenderer.h"
#include <QDebug>
#include <algorithm>

// Billboard 顶点着色器：实例化渲染，每个 Gaussian 一个 billboard quad
static const char *BILL_VERT = R"(
#version 330 core
// Quad 顶点（本地空间 [-0.5, 0.5]）
layout(location = 0) in vec2 aQuad;
// 实例化数据
layout(location = 1) in vec3 iPos;
layout(location = 2) in vec3 iColor;
layout(location = 3) in float iOpacity;
layout(location = 4) in vec3 iScale;

uniform mat4 uMVP;
uniform mat4 uView;
uniform float uSplatScale;
uniform float uAlphaScale;

out vec4 vColor;
out vec2 vUV;

void main() {
    // 提取相机右向量和上向量（从 View 矩阵列）
    vec3 right = vec3(uView[0][0], uView[1][0], uView[2][0]);
    vec3 up    = vec3(uView[0][1], uView[1][1], uView[2][1]);

    float sx = iScale.x * uSplatScale;
    float sy = iScale.y * uSplatScale;

    vec3 worldPos = iPos
        + right * aQuad.x * sx
        + up    * aQuad.y * sy;

    gl_Position = uMVP * vec4(worldPos, 1.0);
    vColor = vec4(iColor, clamp(iOpacity * uAlphaScale, 0.0, 1.0));
    vUV = aQuad + vec2(0.5);
}
)";

static const char *BILL_FRAG = R"(
#version 330 core
in vec4 vColor;
in vec2 vUV;
out vec4 fragColor;
void main() {
    // 高斯形状：中心最不透明，边缘衰减
    vec2 d = vUV - vec2(0.5);
    float r2 = dot(d, d) * 4.0; // [0,1] 范围
    if (r2 > 1.0) discard;
    float alpha = vColor.a * exp(-r2 * 2.0);
    fragColor = vec4(vColor.rgb, alpha);
}
)";

GaussianBillboardRenderer::~GaussianBillboardRenderer() {
    cleanup();
}

void GaussianBillboardRenderer::initialize() {
    initializeOpenGLFunctions();
    m_shader.addShaderFromSourceCode(QOpenGLShader::Vertex, BILL_VERT);
    m_shader.addShaderFromSourceCode(QOpenGLShader::Fragment, BILL_FRAG);
    m_shader.link();

    // 单位四边形（两个三角形）
    float quad[] = {
        -0.5f, -0.5f,
         0.5f, -0.5f,
         0.5f,  0.5f,
        -0.5f, -0.5f,
         0.5f,  0.5f,
        -0.5f,  0.5f,
    };

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_quadVbo);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);

    // 绑定 quad 顶点
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    // 实例化数据 VBO（先分配空间）
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    // pos (3), color (3), opacity (1), scale (3) = 10 floats per instance
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)0);
    glVertexAttribDivisor(1, 1);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(3 * sizeof(float)));
    glVertexAttribDivisor(2, 1);

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(6 * sizeof(float)));
    glVertexAttribDivisor(3, 1);

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(7 * sizeof(float)));
    glVertexAttribDivisor(4, 1);

    glBindVertexArray(0);
    m_initialized = true;
}

void GaussianBillboardRenderer::upload(const GaussianCloud &cloud) {
    if (!m_initialized) return;

    const auto &pts = cloud.points();
    m_splatCount = pts.size();
    m_splatData.resize(m_splatCount);

    for (int i = 0; i < m_splatCount; ++i) {
        m_splatData[i] = {
            pts[i].position,
            pts[i].color,
            pts[i].opacity,
            pts[i].scale
        };
    }
    // 初始上传（不排序）
    QVector<float> buf;
    buf.reserve(m_splatCount * 10);
    for (const auto &s : m_splatData) {
        buf.append(s.pos.x()); buf.append(s.pos.y()); buf.append(s.pos.z());
        buf.append(s.color.x()); buf.append(s.color.y()); buf.append(s.color.z());
        buf.append(s.opacity);
        buf.append(s.scale.x()); buf.append(s.scale.y()); buf.append(s.scale.z());
    }
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, buf.size() * sizeof(float), buf.constData(), GL_DYNAMIC_DRAW);
}

void GaussianBillboardRenderer::sortByDepth(const QVector3D &cameraPos) {
    // 按到相机距离从远到近排序（确保透明度混合正确）
    std::sort(m_splatData.begin(), m_splatData.end(),
        [&](const SplatData &a, const SplatData &b) {
            return (a.pos - cameraPos).lengthSquared() > (b.pos - cameraPos).lengthSquared();
        });

    QVector<float> buf;
    buf.reserve(m_splatCount * 10);
    for (const auto &s : m_splatData) {
        buf.append(s.pos.x()); buf.append(s.pos.y()); buf.append(s.pos.z());
        buf.append(s.color.x()); buf.append(s.color.y()); buf.append(s.color.z());
        buf.append(s.opacity);
        buf.append(s.scale.x()); buf.append(s.scale.y()); buf.append(s.scale.z());
    }
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, buf.size() * sizeof(float), buf.constData());
}

void GaussianBillboardRenderer::render(const QMatrix4x4 &mvp, const QMatrix4x4 &view,
                                        const QVector3D &cameraPos, float splatScale, float alphaScale) {
    if (!m_initialized || m_splatCount == 0) return;

    // 每帧排序（小数据集可接受，大数据集可降频）
    if (m_splatCount <= 20000) {
        sortByDepth(cameraPos);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE); // 透明物体不写深度

    m_shader.bind();
    m_shader.setUniformValue("uMVP", mvp);
    m_shader.setUniformValue("uView", view);
    m_shader.setUniformValue("uSplatScale", splatScale);
    m_shader.setUniformValue("uAlphaScale", alphaScale);

    glBindVertexArray(m_vao);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, m_splatCount);
    glBindVertexArray(0);

    m_shader.release();
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void GaussianBillboardRenderer::cleanup() {
    if (m_vao) { glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
    if (m_vbo) { glDeleteBuffers(1, &m_vbo); m_vbo = 0; }
    if (m_quadVbo) { glDeleteBuffers(1, &m_quadVbo); m_quadVbo = 0; }
    m_initialized = false;
}
