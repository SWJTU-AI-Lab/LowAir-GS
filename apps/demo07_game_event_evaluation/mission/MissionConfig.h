#pragma once
#include "VirtualTarget.h"
#include "TriggerZone.h"
#include <QString>
#include <QVector3D>
#include <vector>

// 任务配置（从 JSON 加载）
struct MissionConfig {
    QString              missionId;
    QString              scenarioName;
    QString              meshPath;
    QString              gaussianPath;
    QString              replayTrackPath;
    float                replaySpeed = 1.0f;
    bool                 replayLoop  = true;
    int                  maxScore    = 100;

    std::vector<VirtualTarget> targets;
    std::vector<TriggerZone>   zones;

    // 评分权重
    int scoreTargetReached = 20;
    int scoreZoneCoverage  = 30;
    int scorePathEfficiency= 20;
    int scoreTimeEfficiency= 15;
    int scoreEnergyEstimate= 15;

    // 从 JSON 文件加载
    static bool loadFromFile(const QString& path, MissionConfig& cfg);
};
