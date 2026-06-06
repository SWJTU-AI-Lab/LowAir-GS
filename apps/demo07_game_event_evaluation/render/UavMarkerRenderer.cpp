#include "UavMarkerRenderer.h"
#include <cmath>

static const char* UAV_VERT = R"(
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aColor;
uniform mat4 uMVP;
out vec3 vColor;
void main(){ gl_Position = uMVP * vec4(aPos, 1.0); vColor = aColor; }
)";
static const char* UAV_FRAG = R"(
#version 330 core
in vec3 vColor;
out vec4 fragColor;
void main(){ fragColor = vec4(vColor, 1.0); }
)";

UavMarkerRenderer::~UavMarkerRenderer() { cleanup(); }

void UavMarkerRenderer::initialize() {
    initializeOpenGLFunctions();
    m_prog.addShaderFromSourceCode(QOpenGLShader::Vertex,   UAV_VERT);
    m_prog.addShaderFromSourceCode(QOpenGLShader::Fragment, UAV_FRAG);
    m_prog.link();

    // 简单十字形标记（pos + color）
    float s = 0.5f;
    float verts[] = {
        // 十字横
        -s, 0, 0,  1.0f, 0.5f, 0.0f,
         s, 0, 0,  1.0f, 0.5f, 0.0f,
        // 十字纵
        0, 0, -s,  1.0f, 0.5f, 0.0f,
        0, 0,  s,  1.0f, 0.5f, 0.0f,
        // 航向箭头
        0, 0,  0,  1.0f, 1.0f, 0.0f,
        0, 0, -1.2f, 1.0f, 1.0f, 0.0f,
    };

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    m_initialized = true;
}

void UavMarkerRenderer::render(const QMatrix4x4& mvp, const QVector3D& pos, float yawDeg) {
    if (!m_initialized) return;
    QMatrix4x4 model;
    model.translate(pos);
    model.rotate(yawDeg, 0, 1, 0);

    m_prog.bind();
    m_prog.setUniformValue("uMVP", mvp * model);
    glBindVertexArray(m_vao);
    glLineWidth(2.0f);
    glDrawArrays(GL_LINES, 0, 6);
    glLineWidth(1.0f);
    glBindVertexArray(0);
    m_prog.release();
}

void UavMarkerRenderer::cleanup() {
    if (m_vao) { glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
    if (m_vbo) { glDeleteBuffers(1, &m_vbo);      m_vbo = 0; }
}
