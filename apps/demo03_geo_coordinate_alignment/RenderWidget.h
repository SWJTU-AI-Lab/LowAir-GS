#pragma once

// RenderWidget.h
// OpenGL 三维视口：ENU 局部场景显示

#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include <QTimer>
#include "camera/OrbitCamera.h"
#include "render/GeoSceneRenderer.h"
#include "render/UavMarkerRenderer.h"
#include "render/TrajectoryRenderer.h"
#include "geo/GeoCoordinate.h"

class RenderWidget : public QOpenGLWidget, protected QOpenGLExtraFunctions {
    Q_OBJECT

public:
    explicit RenderWidget(QWidget* parent = nullptr);
    ~RenderWidget();

    void updateUavState(const OpenGLCoord& pos, float yaw_deg);
    void clearTrajectory();
    void resetView();
    void fitToTrajectory();
    QImage takeScreenshot();

    void setShowGrid(bool v);
    void setShowAxis(bool v);
    void setShowOriginMarker(bool v);
    void setShowDistanceRings(bool v);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void wheelEvent(QWheelEvent* e) override;

private:
    OrbitCamera        m_camera;
    GeoSceneRenderer   m_sceneRenderer;
    UavMarkerRenderer  m_uavRenderer;
    TrajectoryRenderer m_trajRenderer;

    QTimer* m_renderTimer = nullptr;
    int     m_width = 800;
    int     m_height = 600;
};
