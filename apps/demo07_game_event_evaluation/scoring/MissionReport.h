#pragma once
#include "../mission/MissionState.h"
#include "../event/EventTimeline.h"
#include "ScoreEvaluator.h"
#include <QString>

// 任务报告导出（JSON + CSV）
class MissionReport {
public:
    // 导出 JSON 报告
    static bool exportJson(const QString& path,
                           const MissionState& state,
                           const ScoreEvaluator& evaluator,
                           const EventTimeline& timeline,
                           const QString& screenshotPath = QString());

    // 导出 CSV 报告
    static bool exportCsv(const QString& path,
                          const MissionState& state,
                          const ScoreEvaluator& evaluator,
                          const EventTimeline& timeline);
};
