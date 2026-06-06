#pragma once

// UavMarkerRenderer.h
// 无人机标记渲染器：绘制无人机当前位置（橙色点）和航向箭头

#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <QVector3D>

class UavMarkerRenderer : protected QOpenGLExtraFunctions {
public:
    UavMarkerRenderer();
    ~UavMarkerRenderer();

    void initialize();
    void render(const QMatrix4x4& view, const QMatrix4x4& proj);

    void setPosition(const QVector3D& pos) { m_pos = pos; m_hasData = true; }
    void setYaw(float yaw_deg)             { m_yaw = yaw_deg; }
    bool hasData() const                   { return m_hasData; }

private:
    void drawMarker(const QMatrix4x4& mvp);
    void drawArrow(const QMatrix4x4& mvp);

    QOpenGLShaderProgram* m_prog = nullptr;
    QVector3D m_pos  = {0, 0, 0};
    float     m_yaw  = 0.0f;
    bool      m_hasData = false;
    bool      m_initialized = false;
};
