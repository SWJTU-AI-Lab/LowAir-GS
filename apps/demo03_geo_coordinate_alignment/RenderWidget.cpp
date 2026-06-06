#include "RenderWidget.h"
#include <QMouseEvent>
#include <QWheelEvent>

RenderWidget::RenderWidget(QWidget* parent)
    : QOpenGLWidget(parent) {
    setMinimumSize(600, 400);
    m_renderTimer = new QTimer(this);
    connect(m_renderTimer, &QTimer::timeout, this, QOverload<>::of(&RenderWidget::update));
    m_renderTimer->start(33);  // ~30fps
}

RenderWidget::~RenderWidget() {}

void RenderWidget::initializeGL() {
    QOpenGLExtraFunctions::initializeOpenGLFunctions();
    glClearColor(0.12f, 0.12f, 0.16f, 1.0f);  // 深色背景
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_sceneRenderer.initialize();
    m_uavRenderer.initialize();
    m_trajRenderer.initialize();
}

void RenderWidget::resizeGL(int w, int h) {
    m_width  = w;
    m_height = h;
    glViewport(0, 0, w, h);
}

void RenderWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float aspect = (m_height > 0) ? static_cast<float>(m_width) / m_height : 1.0f;
    QMatrix4x4 view = m_camera.viewMatrix();
    QMatrix4x4 proj = m_camera.projectionMatrix(aspect);

    m_sceneRenderer.render(view, proj);
    m_trajRenderer.render(view, proj);
    m_uavRenderer.render(view, proj);
}

void RenderWidget::updateUavState(const OpenGLCoord& pos, float yaw_deg) {
    QVector3D glPos(pos.x, pos.y, pos.z);
    m_uavRenderer.setPosition(glPos);
    m_uavRenderer.setYaw(yaw_deg);
    m_trajRenderer.addPoint(glPos);
    update();
}

void RenderWidget::clearTrajectory() {
    m_trajRenderer.clear();
    update();
}

void RenderWidget::resetView() {
    m_camera.reset();
    update();
}

void RenderWidget::fitToTrajectory() {
    m_camera.fitToRadius(100.0f);
    update();
}

QImage RenderWidget::takeScreenshot() {
    return grabFramebuffer();
}

void RenderWidget::setShowGrid(bool v) {
    m_sceneRenderer.setShowGrid(v);
    update();
}

void RenderWidget::setShowAxis(bool v) {
    m_sceneRenderer.setShowAxis(v);
    update();
}

void RenderWidget::setShowOriginMarker(bool v) {
    m_sceneRenderer.setShowOriginMarker(v);
    update();
}

void RenderWidget::setShowDistanceRings(bool v) {
    m_sceneRenderer.setShowDistanceRings(v);
    update();
}

void RenderWidget::mousePressEvent(QMouseEvent* e) {
    m_camera.mousePressEvent(e->button(), e->pos());
}

void RenderWidget::mouseMoveEvent(QMouseEvent* e) {
    m_camera.mouseMoveEvent(e->pos());
    update();
}

void RenderWidget::mouseReleaseEvent(QMouseEvent* e) {
    Q_UNUSED(e)
    m_camera.mouseReleaseEvent();
}

void RenderWidget::wheelEvent(QWheelEvent* e) {
    m_camera.wheelEvent(static_cast<float>(e->angleDelta().y()));
    update();
}
