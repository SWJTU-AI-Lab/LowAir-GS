#include "EventEngine.h"

EventEngine::EventEngine(QObject* parent) : QObject(parent) {}

void EventEngine::setTargets(std::vector<VirtualTarget>* targets) { m_targets = targets; }
void EventEngine::setZones(std::vector<TriggerZone>* zones)       { m_zones   = zones; }
void EventEngine::setTimeline(EventTimeline* timeline)             { m_timeline = timeline; }

void EventEngine::update(const QVector3D& uavPos, float missionTime) {
    // 检测虚拟目标点
    if (m_targets) {
        for (auto& t : *m_targets) {
            if (t.completed) continue;
            float dist = (uavPos - t.position).length();
            if (dist <= t.radius) {
                t.triggered  = true;
                t.completed  = true;
                EventRecord rec;
                rec.type       = EventType::TargetReached;
                rec.targetId   = t.id;
                rec.message    = QString("Reached: %1").arg(t.name);
                rec.scoreDelta = t.score;
                rec.timestamp  = missionTime;
                if (m_timeline) m_timeline->addEvent(rec);
                emit targetTriggered(t.id, t.effectType);
                emit scoreChanged(t.score);
            }
        }
    }

    // 检测触发区域
    if (m_zones) {
        for (auto& z : *m_zones) {
            if (z.completed) continue;
            if (z.contains(uavPos)) {
                if (!z.triggered) {
                    z.triggered = true;
                    EventRecord rec;
                    rec.type       = EventType::ZoneEntered;
                    rec.targetId   = z.id;
                    rec.message    = QString("Entered zone: %1").arg(z.name);
                    rec.scoreDelta = z.score;
                    rec.timestamp  = missionTime;
                    if (m_timeline) m_timeline->addEvent(rec);
                    emit zoneTriggered(z.id, z.effectType);
                    emit scoreChanged(z.score);
                    z.completed = true;
                }
            }
        }
    }
}

void EventEngine::reset() {
    if (m_targets) for (auto& t : *m_targets) { t.triggered = false; t.completed = false; }
    if (m_zones)   for (auto& z : *m_zones)   { z.triggered = false; z.completed = false; }
}
