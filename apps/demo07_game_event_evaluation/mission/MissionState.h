#pragma once
#include <QString>
#include <QDateTime>

// 任务运行状态
enum class MissionStatus {
    Idle,
    Running,
    Paused,
    Completed,
    Failed
};

struct MissionState {
    MissionStatus status     = MissionStatus::Idle;
    QString       missionName;
    QDateTime     startTime;
    QDateTime     endTime;
    float         elapsedSec = 0.0f;
    float         pathLength = 0.0f;
    int           currentScore = 0;
    int           maxScore     = 100;
    int           targetsCompleted = 0;
    int           totalTargets     = 0;
    int           zonesCompleted   = 0;
    int           totalZones       = 0;
    int           eventsTriggered  = 0;
    float         batteryEstimate  = 100.0f;   // 百分比

    QString statusString() const {
        switch (status) {
            case MissionStatus::Idle:      return "Idle";
            case MissionStatus::Running:   return "Running";
            case MissionStatus::Paused:    return "Paused";
            case MissionStatus::Completed: return "Completed";
            case MissionStatus::Failed:    return "Failed";
        }
        return "Unknown";
    }
};
