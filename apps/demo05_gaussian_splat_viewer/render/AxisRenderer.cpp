#include "AxisRenderer.h"

static const char *AXIS_VERT = R"(
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aColor;
uniform mat4 uMVP;
uniform float uLength;
out vec3 vColor;
void main() {
    gl_Position = uMVP * vec4(aPos * uLength, 1.0);
    vColor = aColor;
}
)";
static const char *AXIS_FRAG = R"(
#version 330 core
in vec3 vColor;
out vec4 fragColor;
void main() { fragColor = vec4(vColor, 1.0); }
)";

void AxisRenderer::initialize() {
    initializeOpenGLFunctions();
    m_shader.addShaderFromSourceCode(QOpenGLShader::Vertex, AXIS_VERT);
    m_shader.addShaderFromSourceCode(QOpenGLShader::Fragment, AXIS_FRAG);
    m_shader.link();

    // X=红, Y=绿, Z=蓝
    float verts[] = {
        0,0,0, 1,0,0,  1,0,0, 1,0,0,
        0,0,0, 0,1,0,  0,1,0, 0,1,0,
        0,0,0, 0,0,1,  0,0,1, 0,0,1,
    };
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    glBindVertexArray(0);
    m_initialized = true;
}

void AxisRenderer::render(const QMatrix4x4 &mvp, float length) {
    if (!m_initialized) return;
    glLineWidth(2.0f);
    m_shader.bind();
    m_shader.setUniformValue("uMVP", mvp);
    m_shader.setUniformValue("uLength", length);
    glBindVertexArray(m_vao);
    glDrawArrays(GL_LINES, 0, 6);
    glBindVertexArray(0);
    m_shader.release();
    glLineWidth(1.0f);
}

void AxisRenderer::cleanup() {
    if (m_vao) { glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
    if (m_vbo) { glDeleteBuffers(1, &m_vbo); m_vbo = 0; }
}
