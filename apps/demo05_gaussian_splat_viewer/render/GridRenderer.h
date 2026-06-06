#pragma once
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>

class GridRenderer : protected QOpenGLExtraFunctions {
public:
    void initialize();
    void render(const QMatrix4x4 &mvp, float gridSize = 10.0f, int divisions = 20);
    void cleanup();
private:
    bool m_initialized = false;
    GLuint m_vao = 0, m_vbo = 0;
    int m_lineCount = 0;
    QOpenGLShaderProgram m_shader;
    float m_lastGridSize = 0;
    int m_lastDivisions = 0;
    void buildGrid(float gridSize, int divisions);
};
