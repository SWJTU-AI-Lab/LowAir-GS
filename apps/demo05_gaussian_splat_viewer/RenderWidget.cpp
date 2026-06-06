#include "RenderWidget.h"
#include <QOpenGLContext>
#include <QPainter>
#include <QDateTime>
#include <QImage>
#include <QDebug>

RenderWidget::RenderWidget(QWidget *parent) : QOpenGLWidget(parent) {
    QSurfaceFormat fmt;
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setSamples(4);
    setFormat(fmt);

    connect(&m_renderTimer, &QTimer::timeout, this, [this]() { update(); });
    m_renderTimer.start(16); // ~60fps
}

RenderWidget::~RenderWidget() {
    makeCurrent();
    m_pointRenderer.cleanup();
    m_billboardRenderer.cleanup();
    m_gridRenderer.cleanup();
    m_axisRenderer.cleanup();
    m_bboxRenderer.cleanup();
    doneCurrent();
}

void RenderWidget::initializeGL() {
    initializeOpenGLFunctions();
    glClearColor(0.12f, 0.12f, 0.15f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    m_pointRenderer.initialize();
    m_billboardRenderer.initialize();
    m_gridRenderer.initialize();
    m_axisRenderer.initialize();
    m_bboxRenderer.initialize();
}

void RenderWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
    m_camera.setAspect((float)w / (float)h);
}

void RenderWidget::paintGL() {
    m_fps.tick();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QMatrix4x4 mvp = m_camera.mvpMatrix();
    QMatrix4x4 view = m_camera.viewMatrix();
    QVector3D camPos = m_camera.position();

    // 如果有新点云数据需要上传
    if (m_cloudDirty && m_cloud) {
        m_pointRenderer.upload(*m_cloud);
        m_billboardRenderer.upload(*m_cloud);
        if (m_cloud->count() > 0) {
            m_bboxRenderer.setBox(m_cloud->bboxMin(), m_cloud->bboxMax());
            m_bboxSet = true;
        }
        m_cloudDirty = false;
    }

    // 渲染网格
    if (m_showGrid) {
        float gridSize = m_cloud ? qMax(m_cloud->bboxSize().length() * 0.6f, 5.0f) : 5.0f;
        m_gridRenderer.render(mvp, gridSize, 20);
    }

    // 渲染坐标轴
    if (m_showAxis) {
        float axisLen = m_cloud ? qMax(m_cloud->bboxSize().length() * 0.3f, 1.0f) : 1.0f;
        m_axisRenderer.render(mvp, axisLen);
    }

    // 渲染 Gaussian 点云
    if (m_cloud && m_cloud->count() > 0) {
        if (m_renderMode == RenderMode::Point) {
            m_pointRenderer.render(mvp, m_pointSize, false);
        } else if (m_renderMode == RenderMode::DebugColor) {
            m_pointRenderer.render(mvp, m_pointSize, true);
        } else { // Splat
            m_billboardRenderer.render(mvp, view, camPos, m_splatScale, m_alphaScale);
        }
    }

    // 渲染包围盒
    if (m_showBBox && m_bboxSet) {
        m_bboxRenderer.render(mvp);
    }

    // FPS 更新
    emit fpsUpdated(m_fps.fps(), m_fps.frameTimeMs());
}

void RenderWidget::loadCloud(std::shared_ptr<GaussianCloud> cloud) {
    m_cloud = cloud;
    m_cloudDirty = true;
    m_bboxSet = false;
    update();
}

void RenderWidget::setRenderMode(RenderMode mode) { m_renderMode = mode; update(); }
void RenderWidget::setShowGrid(bool v) { m_showGrid = v; update(); }
void RenderWidget::setShowAxis(bool v) { m_showAxis = v; update(); }
void RenderWidget::setShowBBox(bool v) { m_showBBox = v; update(); }
void RenderWidget::setPointSize(float v) { m_pointSize = v; update(); }
void RenderWidget::setSplatScale(float v) { m_splatScale = v; update(); }
void RenderWidget::setAlphaScale(float v) { m_alphaScale = v; update(); }

void RenderWidget::resetView() {
    m_camera.reset();
    update();
}

void RenderWidget::fitToCloud() {
    if (!m_cloud || m_cloud->count() == 0) return;
    m_camera.setTarget(m_cloud->center());
    float diag = m_cloud->bboxSize().length();
    m_camera.setDistance(diag * 1.5f);
    update();
}

void RenderWidget::saveScreenshot(const QString &path) {
    QImage img = grabFramebuffer();
    img.save(path);
    qDebug() << "[RenderWidget] Screenshot saved to" << path;
}

float RenderWidget::currentFps() const { return m_fps.fps(); }
float RenderWidget::currentFrameTimeMs() const { return m_fps.frameTimeMs(); }

void RenderWidget::mousePressEvent(QMouseEvent *e) {
    m_lastMousePos = e->pos();
    m_pressedButton = e->button();
}

void RenderWidget::mouseMoveEvent(QMouseEvent *e) {
    QPoint delta = e->pos() - m_lastMousePos;
    m_lastMousePos = e->pos();
    if (m_pressedButton == Qt::LeftButton) {
        m_camera.orbit(delta.x() * 0.4f, -delta.y() * 0.4f);
    } else if (m_pressedButton == Qt::RightButton || m_pressedButton == Qt::MiddleButton) {
        m_camera.pan(delta.x(), delta.y());
    }
    update();
}

void RenderWidget::mouseReleaseEvent(QMouseEvent *) {
    m_pressedButton = Qt::NoButton;
}

void RenderWidget::wheelEvent(QWheelEvent *e) {
    m_camera.zoom(e->angleDelta().y() / 120.0f);
    update();
}
