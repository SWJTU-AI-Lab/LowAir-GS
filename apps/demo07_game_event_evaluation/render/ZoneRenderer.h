#pragma once
#include "../mission/TriggerZone.h"
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <vector>

// 触发区域渲染器（半透明线框长方体）
class ZoneRenderer : public QOpenGLExtraFunctions {
public:
    ZoneRenderer() = default;
    ~ZoneRenderer();

    void initialize();
    void render(const QMatrix4x4& mvp, const std::vector<TriggerZone>& zones);
    void cleanup();

private:
    void buildBoxGeometry();

    QOpenGLShaderProgram m_prog;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_ibo = 0;
    int    m_indexCount = 0;
    bool   m_initialized = false;
};
