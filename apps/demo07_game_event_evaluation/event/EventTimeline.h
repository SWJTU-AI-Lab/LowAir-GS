#pragma once
#include "EventRecord.h"
#include <QObject>
#include <vector>

// 事件时间线：存储并管理所有已触发事件
class EventTimeline : public QObject {
    Q_OBJECT
public:
    explicit EventTimeline(QObject* parent = nullptr);

    void addEvent(const EventRecord& rec);
    void clear();
    const std::vector<EventRecord>& events() const { return m_events; }
    int  count() const { return (int)m_events.size(); }

signals:
    void eventAdded(const EventRecord& rec);

private:
    std::vector<EventRecord> m_events;
    int m_nextId = 1;
};
