#include "EventTimeline.h"

EventTimeline::EventTimeline(QObject* parent) : QObject(parent) {}

void EventTimeline::addEvent(const EventRecord& rec) {
    EventRecord r = rec;
    r.id = m_nextId++;
    r.wallTime = QDateTime::currentDateTime();
    m_events.push_back(r);
    emit eventAdded(r);
}

void EventTimeline::clear() {
    m_events.clear();
    m_nextId = 1;
}
