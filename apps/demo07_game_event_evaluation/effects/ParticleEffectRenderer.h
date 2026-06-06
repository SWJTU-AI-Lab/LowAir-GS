#pragma once
#include "Effect.h"
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <vector>

struct Particle {
    QVector3D pos;
    QVector3D vel;
    float     life    = 1.0f;
    float     maxLife = 1.0f;
};

// 粒子特效渲染器（任务完成时触发）
class ParticleEffectRenderer : public QOpenGLExtraFunctions {
public:
    ParticleEffectRenderer() = default;
    ~ParticleEffectRenderer();

    void initialize();
    void spawnBurst(const QVector3D& center, int count = 60);
    void update(float dt);
    void render(const QMatrix4x4& mvp);
    void cleanup();
    bool hasActiveParticles() const { return !m_particles.empty(); }

private:
    QOpenGLShaderProgram m_prog;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    std::vector<Particle> m_particles;
    bool m_initialized = false;
};
