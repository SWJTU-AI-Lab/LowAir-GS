#include "RenderWidget.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPainter>

RenderWidget::RenderWidget(QWidget* parent) : QOpenGLWidget(parent) {
    m_camera.setDistance(30.0f);
    m_camera.orbit(0.0f, 30.0f);
    connect(&m_renderTimer, &QTimer::timeout, this, [this](){
        update();
        emit fpsUpdated(m_fps.fps());
    });
    m_renderTimer.start(16);
}

RenderWidget::~RenderWidget() {
    makeCurrent();
    m_grid.cleanup();
    m_axis.cleanup();
    m_targetRdr.cleanup();
    m_zoneRdr.cleanup();
    m_trajRdr.cleanup();
    m_uavRdr.cleanup();
    m_glowRdr.cleanup();
    m_partRdr.cleanup();
    doneCurrent();
}

void RenderWidget::initializeGL() {
    initializeOpenGLFunctions();
    glClearColor(0.08f, 0.09f, 0.12f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_grid.initialize(50.0f, 20);
    m_axis.initialize(10.0f);
    m_targetRdr.initialize();
    m_zoneRdr.initialize();
    m_trajRdr.initialize();
    m_uavRdr.initialize();
    m_glowRdr.initialize();
    m_partRdr.initialize();
}

void RenderWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
}

void RenderWidget::paintGL() {
    m_fps.tick();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float aspect = (float)width() / std::max(height(), 1);
    QMatrix4x4 proj, view;
    proj.perspective(45.0f, aspect, 0.1f, 1000.0f);
    view = m_camera.viewMatrix();
    QMatrix4x4 mvp = proj * view;

    m_grid.render(mvp);
    m_axis.render(mvp);
    m_zoneRdr.render(mvp, m_zones);
    m_targetRdr.render(mvp, m_targets);
    m_trajRdr.render(mvp);
    m_uavRdr.render(mvp, m_uavPos, m_uavYaw);
    m_glowRdr.render(mvp, m_effects);
    m_partRdr.render(mvp);

    // HUD（QPainter 叠加）
    QPainter painter(this);
    m_hud.render(painter, width(), height(),
                 m_score, m_maxScore, m_elapsed, m_battery,
                 m_events, m_status);
    // Label Popup 弹窗（右上角）
    m_hud.renderPopups(painter, width(), height(), m_popups);
    painter.end();
}

void RenderWidget::setTargets(const std::vector<VirtualTarget>& targets) {
    m_targets = targets;
}

void RenderWidget::setZones(const std::vector<TriggerZone>& zones) {
    m_zones = zones;
}

void RenderWidget::setUavPosition(const QVector3D& pos, float yawDeg) {
    m_uavPos = pos;
    m_uavYaw = yawDeg;
    m_trajRdr.addPoint(pos);
}

void RenderWidget::clearTrajectory() {
    m_trajRdr.clear();
}

void RenderWidget::spawnEffect(const Effect& e) {
    m_effects.push_back(e);
    if (e.type == EffectType::Particle) {
        m_partRdr.spawnBurst(e.position, 60);
    }
}

void RenderWidget::updateParticles(float dt) {
    m_partRdr.update(dt);
    m_popups.update(dt);
    // 更新 effect 生命周期
    for (auto& ef : m_effects) {
        ef.elapsed += dt;
        if (ef.elapsed >= ef.lifetime) ef.active = false;
    }
    m_effects.erase(
        std::remove_if(m_effects.begin(), m_effects.end(),
                       [](const Effect& e){ return !e.active; }),
        m_effects.end());
}

void RenderWidget::setHudData(int score, int maxScore, float elapsed, float battery,
                               int events, const QString& status) {
    m_score    = score;
    m_maxScore = maxScore;
    m_elapsed  = elapsed;
    m_battery  = battery;
    m_events   = events;
    m_status   = status;
}

void RenderWidget::pushPopup(const QString& text) {
    m_popups.push(text);
}

void RenderWidget::setPathHighlight(int upToIndex) {
    m_trajRdr.setHighlightUpTo(upToIndex);
}

void RenderWidget::mousePressEvent(QMouseEvent* e) {
    m_lastMousePos = e->pos();
}

void RenderWidget::mouseMoveEvent(QMouseEvent* e) {
    QPoint delta = e->pos() - m_lastMousePos;
    m_lastMousePos = e->pos();
    if (e->buttons() & Qt::LeftButton) {
        m_camera.orbit(delta.x() * 0.4f, delta.y() * 0.4f);
    } else if (e->buttons() & Qt::RightButton) {
        m_camera.pan(delta.x() * 0.05f, -delta.y() * 0.05f);
    }
}

void RenderWidget::wheelEvent(QWheelEvent* e) {
    m_camera.zoom(-e->angleDelta().y() * 0.02f);
}
