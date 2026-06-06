#pragma once
#include "../mission/VirtualTarget.h"
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <vector>

// 虚拟目标点渲染器（球形标记 + 颜色状态）
class TargetRenderer : public QOpenGLExtraFunctions {
public:
    TargetRenderer() = default;
    ~TargetRenderer();

    void initialize();
    void render(const QMatrix4x4& mvp, const std::vector<VirtualTarget>& targets);
    void cleanup();

private:
    void buildSphereGeometry();

    QOpenGLShaderProgram m_prog;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    int    m_vertexCount = 0;
    bool   m_initialized = false;
};
