#include "ParticleEffectRenderer.h"
#include <cmath>
#include <cstdlib>

static const char* PART_VERT = R"(
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in float aAlpha;
uniform mat4 uMVP;
out float vAlpha;
void main(){
    gl_Position = uMVP * vec4(aPos, 1.0);
    gl_PointSize = 4.0;
    vAlpha = aAlpha;
}
)";

static const char* PART_FRAG = R"(
#version 330 core
in float vAlpha;
out vec4 fragColor;
void main(){
    fragColor = vec4(1.0, 0.85, 0.2, vAlpha);
}
)";

ParticleEffectRenderer::~ParticleEffectRenderer() { cleanup(); }

void ParticleEffectRenderer::initialize() {
    initializeOpenGLFunctions();
    m_prog.addShaderFromSourceCode(QOpenGLShader::Vertex,   PART_VERT);
    m_prog.addShaderFromSourceCode(QOpenGLShader::Fragment, PART_FRAG);
    m_prog.link();

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    // 动态缓冲，每帧上传
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    // pos (3) + alpha (1)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    m_initialized = true;
}

void ParticleEffectRenderer::spawnBurst(const QVector3D& center, int count) {
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.pos = center;
        float theta = ((float)rand() / RAND_MAX) * 2.0f * 3.14159265f;
        float phi   = ((float)rand() / RAND_MAX) * 3.14159265f;
        float speed = 1.0f + ((float)rand() / RAND_MAX) * 3.0f;
        p.vel = QVector3D(sinf(phi)*cosf(theta), cosf(phi), sinf(phi)*sinf(theta)) * speed;
        p.life    = 1.0f + ((float)rand() / RAND_MAX) * 0.5f;
        p.maxLife = p.life;
        m_particles.push_back(p);
    }
}

void ParticleEffectRenderer::update(float dt) {
    for (auto& p : m_particles) {
        p.pos += p.vel * dt;
        p.vel.setY(p.vel.y() - 2.0f * dt);  // 重力
        p.life -= dt;
    }
    m_particles.erase(
        std::remove_if(m_particles.begin(), m_particles.end(),
                       [](const Particle& p){ return p.life <= 0.0f; }),
        m_particles.end());
}

void ParticleEffectRenderer::render(const QMatrix4x4& mvp) {
    if (!m_initialized || m_particles.empty()) return;

    std::vector<float> data;
    data.reserve(m_particles.size() * 4);
    for (const auto& p : m_particles) {
        data.push_back(p.pos.x());
        data.push_back(p.pos.y());
        data.push_back(p.pos.z());
        data.push_back(p.life / p.maxLife);
    }

    m_prog.bind();
    m_prog.setUniformValue("uMVP", mvp);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_DYNAMIC_DRAW);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glDrawArrays(GL_POINTS, 0, (GLsizei)m_particles.size());
    glDisable(GL_PROGRAM_POINT_SIZE);
    glBindVertexArray(0);
    m_prog.release();
}

void ParticleEffectRenderer::cleanup() {
    if (m_vao) { glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
    if (m_vbo) { glDeleteBuffers(1, &m_vbo);      m_vbo = 0; }
}
