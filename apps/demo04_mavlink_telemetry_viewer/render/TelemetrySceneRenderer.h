#pragma once
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <QVector4D>
#include <vector>
#include <QString>

// 遥测场景渲染器
// 负责绘制：地面网格、ENU 坐标轴、地理原点标记、距离环、连接状态文字
class TelemetrySceneRenderer : public QOpenGLExtraFunctions {
public:
    TelemetrySceneRenderer();
    ~TelemetrySceneRenderer();

    void initialize();
    void render(const QMatrix4x4& view, const QMatrix4x4& proj);

    void setShowGrid(bool v)   { m_showGrid   = v; }
    void setShowAxis(bool v)   { m_showAxis   = v; }
    void setShowOrigin(bool v) { m_showOrigin = v; }
    void setShowRings(bool v)  { m_showRings  = v; }

private:
    void buildGrid();
    void buildAxes();
    void buildOriginMarker();
    void buildDistanceRings();
    void drawLines(const std::vector<float>& verts, const QMatrix4x4& mvp,
                   float r, float g, float b, float a = 1.0f, float lineWidth = 1.0f);
    void drawPoints(const std::vector<float>& verts, const QMatrix4x4& mvp,
                    float r, float g, float b, float pointSize = 8.0f);

    QOpenGLShaderProgram* m_prog        = nullptr;
    bool                  m_initialized = false;

    bool m_showGrid   = true;
    bool m_showAxis   = true;
    bool m_showOrigin = true;
    bool m_showRings  = true;

    std::vector<float>              m_gridVerts;
    std::vector<float>              m_axisVerts;
    std::vector<float>              m_originVerts;
    std::vector<std::vector<float>> m_ringVerts;
    std::vector<float>              m_ringRadii;
};
