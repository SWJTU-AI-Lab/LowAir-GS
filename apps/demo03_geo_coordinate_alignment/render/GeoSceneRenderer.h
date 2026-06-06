#pragma once

// GeoSceneRenderer.h
// ENU 地理场景渲染器
// 渲染：地面网格、ENU 坐标轴、地理原点标记、距离环

#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>
#include <vector>

class GeoSceneRenderer : protected QOpenGLExtraFunctions {
public:
    GeoSceneRenderer();
    ~GeoSceneRenderer();

    void initialize();
    void render(const QMatrix4x4& view, const QMatrix4x4& proj);

    void setShowGrid(bool v)          { m_showGrid = v; }
    void setShowAxis(bool v)          { m_showAxis = v; }
    void setShowOriginMarker(bool v)  { m_showOrigin = v; }
    void setShowDistanceRings(bool v) { m_showRings = v; }
    void setMaxRadius(float r)        { m_maxRadius = r; }

private:
    void buildGrid();
    void buildAxes();
    void buildOriginMarker();
    void buildDistanceRings();
    void drawLines(const std::vector<float>& verts,
                   const QMatrix4x4& mvp,
                   float r, float g, float b, float a = 1.0f,
                   float lineWidth = 1.0f);
    void drawPoints(const std::vector<float>& verts,
                    const QMatrix4x4& mvp,
                    float r, float g, float b, float pointSize = 8.0f);

    QOpenGLShaderProgram* m_prog = nullptr;

    // 地面网格顶点（OpenGL 坐标）
    std::vector<float> m_gridVerts;
    // ENU 坐标轴顶点
    std::vector<float> m_axisVerts;
    std::vector<float> m_axisColors;
    // 原点标记
    std::vector<float> m_originVerts;
    // 距离环
    std::vector<std::vector<float>> m_ringVerts;
    std::vector<float> m_ringRadii;

    bool  m_showGrid   = true;
    bool  m_showAxis   = true;
    bool  m_showOrigin = true;
    bool  m_showRings  = true;
    float m_maxRadius  = 200.0f;

    bool m_initialized = false;
};
