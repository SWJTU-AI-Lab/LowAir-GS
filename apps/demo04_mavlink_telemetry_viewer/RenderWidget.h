#pragma once
#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include <QMatrix4x4>
#include <QVector3D>
#include <QTimer>
#include <QMouseEvent>
#include <QWheelEvent>
#include "camera/OrbitCamera.h"
#include "render/TelemetrySceneRenderer.h"
#include "render/UavMarkerRenderer.h"
#include "render/TrajectoryRenderer.h"

// OpenGL 三维遥测场景视口
// 复用 Demo-03 的 ENU 坐标轴、网格、距离环设计
class RenderWidget : public QOpenGLWidget, protected QOpenGLExtraFunctions {
    Q_OBJECT

public:
    explicit RenderWidget(QWidget* parent = nullptr);
    ~RenderWidget() override;

    void updateUavState(const QVector3D& openglPos, float yawDeg, bool connected, bool gpsValid);
    void clearTrajectory();
    void resetView();
    void fitToTrajectory();

    void setShowGrid(bool v);
    void setShowAxis(bool v);
    void setShowOrigin(bool v);
    void setShowRings(bool v);

protected:
    void initializeGL()              override;
    void resizeGL(int w, int h)      override;
    void paintGL()                   override;
    void mousePressEvent(QMouseEvent*)  override;
    void mouseMoveEvent(QMouseEvent*)   override;
    void wheelEvent(QWheelEvent*)       override;

private:
    QTimer*                  m_timer       = nullptr;
    OrbitCamera              m_camera;
    TelemetrySceneRenderer   m_sceneRenderer;
    UavMarkerRenderer        m_uavRenderer;
    TrajectoryRenderer       m_trajRenderer;

    QVector3D m_uavPos       = {0.0f, 0.0f, 0.0f};
    float     m_uavYaw       = 0.0f;
    bool      m_connected    = false;
    bool      m_gpsValid     = false;

    QPoint    m_lastMousePos;
    bool      m_leftDown     = false;
    bool      m_rightDown    = false;

    QMatrix4x4 m_proj;
};
