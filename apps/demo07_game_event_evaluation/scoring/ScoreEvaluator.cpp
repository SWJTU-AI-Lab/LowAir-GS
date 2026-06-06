#include "ScoreEvaluator.h"
#include <algorithm>

ScoreEvaluator::ScoreEvaluator(QObject* parent) : QObject(parent) {}

void ScoreEvaluator::setConfig(const MissionConfig* cfg) {
    m_cfg      = cfg;
    m_maxScore = cfg ? cfg->maxScore : 100;
}

void ScoreEvaluator::setTimeline(const EventTimeline* timeline) {
    m_timeline = timeline;
}

void ScoreEvaluator::evaluate(const MissionState& state) {
    if (!m_cfg) return;

    // 1. 目标完成度得分（Target Completion Score）
    // 公式：S_target = (completed / total) * maxTargetScore
    m_targetScore = 0;
    if (state.totalTargets > 0)
        m_targetScore = (int)((float)state.targetsCompleted / state.totalTargets
                              * m_cfg->scoreTargetReached);

    // 2. 区域覆盖率得分（Zone Coverage Score）
    // 公式：S_zone = (zonesCompleted / totalZones) * maxZoneScore
    m_zoneScore = 0;
    if (state.totalZones > 0)
        m_zoneScore = (int)((float)state.zonesCompleted / state.totalZones
                            * m_cfg->scoreZoneCoverage);

    // 3. 路径效率得分（Path Efficiency Score）
    // 公式：S_path = max(0, maxPathScore * (1 - pathLength / refLength))
    // 参考路径长度 = 100m（教学演示默认值）
    float refLength = 100.0f;
    float pathRatio = (refLength > 0) ? state.pathLength / refLength : 1.0f;
    m_pathScore = (int)(m_cfg->scorePathEfficiency * std::max(0.0f, 1.0f - pathRatio * 0.5f));

    // 4. 时间效率得分（Time Efficiency Score）
    // 公式：S_time = max(0, maxTimeScore * (1 - elapsedSec / refTime))
    // 参考时间 = 120s（教学演示默认值）
    float refTime = 120.0f;
    float timeRatio = (refTime > 0) ? state.elapsedSec / refTime : 1.0f;
    m_timeScore = (int)(m_cfg->scoreTimeEfficiency * std::max(0.0f, 1.0f - timeRatio * 0.5f));

    // 5. 能耗估计得分（Energy Estimate Score）
    // 公式：S_energy = maxEnergyScore * (batteryRemaining / 100)
    m_energyScore = (int)(m_cfg->scoreEnergyEstimate * (state.batteryEstimate / 100.0f));

    // 6. 事件奖励（Event Bonus）
    m_eventBonus = 0;
    if (m_timeline) {
        for (const auto& e : m_timeline->events())
            if (e.scoreDelta > 0) m_eventBonus += e.scoreDelta;
    }

    // 总分（不超过 maxScore）
    m_total = std::min(m_maxScore,
                       m_targetScore + m_zoneScore + m_pathScore +
                       m_timeScore   + m_energyScore);

    emit scoreUpdated(m_total, m_maxScore);
}
