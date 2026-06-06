#pragma once
#include "EventRecord.h"
#include "EventTimeline.h"
#include "../mission/VirtualTarget.h"
#include "../mission/TriggerZone.h"
#include <QObject>
#include <QVector3D>
#include <vector>

// 事件引擎：检测无人机与目标/区域的碰撞，触发事件
class EventEngine : public QObject {
    Q_OBJECT
public:
    explicit EventEngine(QObject* parent = nullptr);

    void setTargets(std::vector<VirtualTarget>* targets);
    void setZones(std::vector<TriggerZone>* zones);
    void setTimeline(EventTimeline* timeline);

    // 每帧调用，传入无人机当前位置和任务时间（秒）
    void update(const QVector3D& uavPos, float missionTime);

    void reset();

signals:
    void targetTriggered(const QString& targetId, const QString& effectType);
    void zoneTriggered(const QString& zoneId, const QString& effectType);
    void scoreChanged(int delta);

private:
    std::vector<VirtualTarget>* m_targets = nullptr;
    std::vector<TriggerZone>*   m_zones   = nullptr;
    EventTimeline*              m_timeline = nullptr;
};
