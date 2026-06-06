#include "BoundingBoxRenderer.h"

static const char *BB_VERT = R"(
#version 330 core
layout(location=0) in vec3 aPos;
uniform mat4 uMVP;
void main() { gl_Position = uMVP * vec4(aPos, 1.0); }
)";
static const char *BB_FRAG = R"(
#version 330 core
out vec4 fragColor;
void main() { fragColor = vec4(1.0, 0.8, 0.0, 1.0); } // 黄色包围盒
)";

void BoundingBoxRenderer::initialize() {
    initializeOpenGLFunctions();
    m_shader.addShaderFromSourceCode(QOpenGLShader::Vertex, BB_VERT);
    m_shader.addShaderFromSourceCode(QOpenGLShader::Fragment, BB_FRAG);
    m_shader.link();
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);
    m_initialized = true;
}

void BoundingBoxRenderer::setBox(const QVector3D &bmin, const QVector3D &bmax) {
    if (!m_initialized) return;
    float x0=bmin.x(), y0=bmin.y(), z0=bmin.z();
    float x1=bmax.x(), y1=bmax.y(), z1=bmax.z();
    float verts[] = {
        x0,y0,z0, x1,y0,z0, x1,y1,z0, x0,y1,z0,
        x0,y0,z1, x1,y0,z1, x1,y1,z1, x0,y1,z1,
    };
    unsigned short idx[] = {
        0,1, 1,2, 2,3, 3,0, // 底面
        4,5, 5,6, 6,7, 7,4, // 顶面
        0,4, 1,5, 2,6, 3,7  // 侧边
    };
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
    glBindVertexArray(0);
}

void BoundingBoxRenderer::render(const QMatrix4x4 &mvp) {
    if (!m_initialized) return;
    m_shader.bind();
    m_shader.setUniformValue("uMVP", mvp);
    glBindVertexArray(m_vao);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_SHORT, nullptr);
    glBindVertexArray(0);
    m_shader.release();
}

void BoundingBoxRenderer::cleanup() {
    if (m_vao) { glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
    if (m_vbo) { glDeleteBuffers(1, &m_vbo); m_vbo = 0; }
    if (m_ebo) { glDeleteBuffers(1, &m_ebo); m_ebo = 0; }
}
