#pragma once
#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include <QMatrix4x4>
#include <QTimer>
#include <memory>

#include "camera/OrbitCamera.h"
#include "perf/FpsCounter.h"
#include "render/GridRenderer.h"
#include "render/AxisRenderer.h"
#include "render/TargetRenderer.h"
#include "render/ZoneRenderer.h"
#include "render/TrajectoryRenderer.h"
#include "render/UavMarkerRenderer.h"
#include "render/HudRenderer.h"
#include "effects/GlowEffectRenderer.h"
#include "effects/ParticleEffectRenderer.h"
#include "mission/VirtualTarget.h"
#include "mission/TriggerZone.h"
#include "effects/Effect.h"

class RenderWidget : public QOpenGLWidget, public QOpenGLExtraFunctions {
    Q_OBJECT
public:
    explicit RenderWidget(QWidget* parent = nullptr);
    ~RenderWidget() override;

    // 场景数据设置
    void setTargets(const std::vector<VirtualTarget>& targets);
    void setZones(const std::vector<TriggerZone>& zones);
    void setUavPosition(const QVector3D& pos, float yawDeg);
    void addTrajectoryPoint(const QVector3D& p);
    void clearTrajectory();
    void spawnEffect(const Effect& e);
    void updateParticles(float dt);

    // HUD 数据
    void setHudData(int score, int maxScore, float elapsed, float battery,
                    int events, const QString& status);

    float fps() const { return m_fps.fps(); }

signals:
    void fpsUpdated(float fps);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void wheelEvent(QWheelEvent* e) override;

private:
    OrbitCamera          m_camera;
    FpsCounter           m_fps;
    GridRenderer         m_grid;
    AxisRenderer         m_axis;
    TargetRenderer       m_targetRdr;
    ZoneRenderer         m_zoneRdr;
    TrajectoryRenderer   m_trajRdr;
    UavMarkerRenderer    m_uavRdr;
    HudRenderer          m_hud;
    GlowEffectRenderer   m_glowRdr;
    ParticleEffectRenderer m_partRdr;

    std::vector<VirtualTarget> m_targets;
    std::vector<TriggerZone>   m_zones;
    std::vector<Effect>        m_effects;

    QVector3D m_uavPos;
    float     m_uavYaw = 0.0f;

    // HUD 数据
    int     m_score    = 0;
    int     m_maxScore = 100;
    float   m_elapsed  = 0.0f;
    float   m_battery  = 100.0f;
    int     m_events   = 0;
    QString m_status   = "Idle";

    QPoint m_lastMousePos;
    QTimer m_renderTimer;
};
