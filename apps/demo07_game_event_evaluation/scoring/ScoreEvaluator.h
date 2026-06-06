#pragma once
#include "ScoreRule.h"
#include "../mission/MissionConfig.h"
#include "../mission/MissionState.h"
#include "../event/EventTimeline.h"
#include <QObject>

// 评分计算器：根据任务状态和事件时间线计算分项得分和总分
class ScoreEvaluator : public QObject {
    Q_OBJECT
public:
    explicit ScoreEvaluator(QObject* parent = nullptr);

    void setConfig(const MissionConfig* cfg);
    void setTimeline(const EventTimeline* timeline);

    // 计算当前分数（实时调用）
    void evaluate(const MissionState& state);

    // 分项得分
    int targetCompletionScore() const { return m_targetScore; }
    int zoneCoverageScore()     const { return m_zoneScore; }
    int pathEfficiencyScore()   const { return m_pathScore; }
    int timeEfficiencyScore()   const { return m_timeScore; }
    int energyEstimateScore()   const { return m_energyScore; }
    int eventBonus()            const { return m_eventBonus; }
    int totalScore()            const { return m_total; }
    int maxScore()              const { return m_maxScore; }

signals:
    void scoreUpdated(int total, int maxScore);

private:
    const MissionConfig*  m_cfg      = nullptr;
    const EventTimeline*  m_timeline = nullptr;

    int m_targetScore = 0;
    int m_zoneScore   = 0;
    int m_pathScore   = 0;
    int m_timeScore   = 0;
    int m_energyScore = 0;
    int m_eventBonus  = 0;
    int m_total       = 0;
    int m_maxScore    = 100;
};
