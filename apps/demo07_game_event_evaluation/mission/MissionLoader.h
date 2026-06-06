#pragma once
#include "MissionConfig.h"
#include "VirtualTarget.h"
#include "TriggerZone.h"
#include <QString>
#include <vector>

// 任务加载器：从独立 JSON 文件加载目标和区域
class MissionLoader {
public:
    static bool loadTargets(const QString& path, std::vector<VirtualTarget>& targets);
    static bool loadZones(const QString& path, std::vector<TriggerZone>& zones);
};
