#include "AxisRenderer.h"

static const char* AXIS_VERT = R"(
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aColor;
uniform mat4 uMVP;
out vec3 vColor;
void main(){ gl_Position = uMVP * vec4(aPos, 1.0); vColor = aColor; }
)";
static const char* AXIS_FRAG = R"(
#version 330 core
in vec3 vColor;
out vec4 fragColor;
void main(){ fragColor = vec4(vColor, 1.0); }
)";

AxisRenderer::~AxisRenderer() { cleanup(); }

void AxisRenderer::initialize(float length) {
    initializeOpenGLFunctions();
    m_prog.addShaderFromSourceCode(QOpenGLShader::Vertex,   AXIS_VERT);
    m_prog.addShaderFromSourceCode(QOpenGLShader::Fragment, AXIS_FRAG);
    m_prog.link();

    float verts[] = {
        0,0,0, 1,0,0,  length,0,0, 1,0,0,  // X 红
        0,0,0, 0,1,0,  0,length,0, 0,1,0,  // Y 绿
        0,0,0, 0,0,1,  0,0,length, 0,0,1   // Z 蓝
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

void AxisRenderer::render(const QMatrix4x4& mvp) {
    if (!m_initialized) return;
    m_prog.bind();
    m_prog.setUniformValue("uMVP", mvp);
    glBindVertexArray(m_vao);
    glDrawArrays(GL_LINES, 0, 6);
    glBindVertexArray(0);
    m_prog.release();
}

void AxisRenderer::cleanup() {
    if (m_vao) { glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
    if (m_vbo) { glDeleteBuffers(1, &m_vbo);      m_vbo = 0; }
}
