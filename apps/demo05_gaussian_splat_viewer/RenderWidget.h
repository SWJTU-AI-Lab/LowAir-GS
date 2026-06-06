#pragma once
#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include <QTimer>
#include <QMouseEvent>
#include <QWheelEvent>
#include <memory>
#include "gaussian/GaussianCloud.h"
#include "render/GaussianPointRenderer.h"
#include "render/GaussianBillboardRenderer.h"
#include "render/GridRenderer.h"
#include "render/AxisRenderer.h"
#include "render/BoundingBoxRenderer.h"
#include "camera/OrbitCamera.h"
#include "perf/FpsCounter.h"

enum class RenderMode { Point, Splat, DebugColor };

class RenderWidget : public QOpenGLWidget, protected QOpenGLExtraFunctions {
    Q_OBJECT
public:
    explicit RenderWidget(QWidget *parent = nullptr);
    ~RenderWidget() override;

    void loadCloud(std::shared_ptr<GaussianCloud> cloud);
    void setRenderMode(RenderMode mode);
    void setShowGrid(bool v);
    void setShowAxis(bool v);
    void setShowBBox(bool v);
    void setPointSize(float v);
    void setSplatScale(float v);
    void setAlphaScale(float v);
    void resetView();
    void fitToCloud();
    void saveScreenshot(const QString &path);

    float currentFps() const;
    float currentFrameTimeMs() const;
    RenderMode renderMode() const { return m_renderMode; }

signals:
    void fpsUpdated(float fps, float frameMs);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;

private:
    std::shared_ptr<GaussianCloud> m_cloud;
    RenderMode m_renderMode = RenderMode::Splat;
    bool m_showGrid = true;
    bool m_showAxis = true;
    bool m_showBBox = true;
    float m_pointSize = 3.0f;
    float m_splatScale = 1.0f;
    float m_alphaScale = 1.0f;

    GaussianPointRenderer m_pointRenderer;
    GaussianBillboardRenderer m_billboardRenderer;
    GridRenderer m_gridRenderer;
    AxisRenderer m_axisRenderer;
    BoundingBoxRenderer m_bboxRenderer;
    OrbitCamera m_camera;
    FpsCounter m_fps;
    QTimer m_renderTimer;

    QPoint m_lastMousePos;
    Qt::MouseButton m_pressedButton = Qt::NoButton;

    bool m_cloudDirty = false;
    bool m_bboxSet = false;
};
