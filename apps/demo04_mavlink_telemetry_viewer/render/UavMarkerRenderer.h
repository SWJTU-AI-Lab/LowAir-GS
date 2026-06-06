#pragma once
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <QVector3D>
#include <vector>

// 无人机标记渲染器
// 绘制橙色方块（无人机位置）+ 青色航向箭头
class UavMarkerRenderer : public QOpenGLExtraFunctions {
public:
    UavMarkerRenderer();
    ~UavMarkerRenderer();

    void initialize();
    void render(const QMatrix4x4& view, const QMatrix4x4& proj,
                const QVector3D& pos, float yawDeg, bool connected, bool gpsValid);

private:
    void drawLines(const std::vector<float>& verts, const QMatrix4x4& mvp,
                   float r, float g, float b, float a = 1.0f, float lineWidth = 2.0f);
    void drawPoints(const std::vector<float>& verts, const QMatrix4x4& mvp,
                    float r, float g, float b, float pointSize = 14.0f);

    QOpenGLShaderProgram* m_prog        = nullptr;
    bool                  m_initialized = false;
};
