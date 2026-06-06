#include "RenderWidget.h"
#include <QOpenGLContext>

RenderWidget::RenderWidget(QWidget* parent) : QOpenGLWidget(parent) {
    setMinimumSize(640, 480);
    m_timer = new QTimer(this);
    m_timer->setInterval(33);  // ~30 FPS
    connect(m_timer, &QTimer::timeout, this, QOverload<>::of(&RenderWidget::update));
    m_timer->start();
}

RenderWidget::~RenderWidget() {}

void RenderWidget::initializeGL() {
    QOpenGLExtraFunctions::initializeOpenGLFunctions();
    glClearColor(0.10f, 0.10f, 0.12f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_sceneRenderer.initialize();
    m_uavRenderer.initialize();
    m_trajRenderer.initialize();
}

void RenderWidget::resizeGL(int w, int h) {
    m_proj.setToIdentity();
    m_proj.perspective(45.0f, static_cast<float>(w) / std::max(h, 1), 0.5f, 5000.0f);
}

void RenderWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    QMatrix4x4 view = m_camera.viewMatrix();
    m_sceneRenderer.render(view, m_proj);
    m_trajRenderer.render(view, m_proj);
    m_uavRenderer.render(view, m_proj, m_uavPos, m_uavYaw, m_connected, m_gpsValid);
}

void RenderWidget::updateUavState(const QVector3D& openglPos, float yawDeg, bool connected, bool gpsValid) {
    m_uavPos   = openglPos;
    m_uavYaw   = yawDeg;
    m_connected = connected;
    m_gpsValid  = gpsValid;
    if (connected && gpsValid) {
        m_trajRenderer.addPoint(openglPos);
    }
}

void RenderWidget::clearTrajectory() {
    m_trajRenderer.clear();
}

void RenderWidget::resetView() {
    m_camera.reset();
}

void RenderWidget::fitToTrajectory() {
    m_camera.fitToRadius(100.0f);
}

void RenderWidget::setShowGrid(bool v)   { m_sceneRenderer.setShowGrid(v); }
void RenderWidget::setShowAxis(bool v)   { m_sceneRenderer.setShowAxis(v); }
void RenderWidget::setShowOrigin(bool v) { m_sceneRenderer.setShowOrigin(v); }
void RenderWidget::setShowRings(bool v)  { m_sceneRenderer.setShowRings(v); }

void RenderWidget::mousePressEvent(QMouseEvent* e) {
    m_lastMousePos = e->pos();
    m_leftDown  = (e->button() == Qt::LeftButton);
    m_rightDown = (e->button() == Qt::RightButton);
}

void RenderWidget::mouseMoveEvent(QMouseEvent* e) {
    QPoint delta = e->pos() - m_lastMousePos;
    m_lastMousePos = e->pos();
    if (m_leftDown)  m_camera.mouseRotate(delta.x(), -delta.y());
    if (m_rightDown) m_camera.mousePan(delta.x(), delta.y());
}

void RenderWidget::wheelEvent(QWheelEvent* e) {
    m_camera.mouseZoom(e->angleDelta().y());
}
