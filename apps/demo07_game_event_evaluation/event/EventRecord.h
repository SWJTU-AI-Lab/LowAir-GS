#pragma once
#include <QString>
#include <QDateTime>

// 事件类型
enum class EventType {
    TargetReached,      // 到达虚拟目标点
    ZoneEntered,        // 进入触发区域
    ZoneExited,         // 离开触发区域
    MissionStart,       // 任务开始
    MissionComplete,    // 任务完成
    MissionTimeout,     // 任务超时
    BoundaryViolation,  // 越出边界
    Custom              // 自定义事件
};

// 单条事件记录
struct EventRecord {
    int       id         = 0;
    EventType type       = EventType::Custom;
    QString   targetId;
    QString   message;
    int       scoreDelta = 0;
    float     timestamp  = 0.0f;   // 任务内相对时间（秒）
    QDateTime wallTime;

    QString typeString() const {
        switch (type) {
            case EventType::TargetReached:     return "TargetReached";
            case EventType::ZoneEntered:       return "ZoneEntered";
            case EventType::ZoneExited:        return "ZoneExited";
            case EventType::MissionStart:      return "MissionStart";
            case EventType::MissionComplete:   return "MissionComplete";
            case EventType::MissionTimeout:    return "MissionTimeout";
            case EventType::BoundaryViolation: return "BoundaryViolation";
            default:                           return "Custom";
        }
    }
};
