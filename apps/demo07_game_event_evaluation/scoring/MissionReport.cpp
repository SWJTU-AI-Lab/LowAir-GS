#include "MissionReport.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextStream>
#include <QDir>

bool MissionReport::exportJson(const QString& path,
                                const MissionState& state,
                                const ScoreEvaluator& evaluator,
                                const EventTimeline& timeline,
                                const QString& screenshotPath) {
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) return false;

    QJsonObject root;
    root["mission_id"]          = state.missionName;
    root["mission_status"]      = state.statusString();
    root["start_time"]          = state.startTime.toString(Qt::ISODate);
    root["end_time"]            = state.endTime.toString(Qt::ISODate);
    root["elapsed_sec"]         = (double)state.elapsedSec;
    root["path_length_m"]       = (double)state.pathLength;
    root["battery_estimate_pct"]= (double)state.batteryEstimate;
    root["targets_completed"]   = state.targetsCompleted;
    root["total_targets"]       = state.totalTargets;
    root["zones_completed"]     = state.zonesCompleted;
    root["total_zones"]         = state.totalZones;
    root["events_triggered"]    = state.eventsTriggered;

    // 分项得分
    QJsonObject scores;
    scores["target_completion"] = evaluator.targetCompletionScore();
    scores["zone_coverage"]     = evaluator.zoneCoverageScore();
    scores["path_efficiency"]   = evaluator.pathEfficiencyScore();
    scores["time_efficiency"]   = evaluator.timeEfficiencyScore();
    scores["energy_estimate"]   = evaluator.energyEstimateScore();
    scores["event_bonus"]       = evaluator.eventBonus();
    scores["final_score"]       = evaluator.totalScore();
    scores["max_score"]         = evaluator.maxScore();
    root["scores"] = scores;

    // 事件列表
    QJsonArray events;
    for (const auto& e : timeline.events()) {
        QJsonObject ev;
        ev["id"]          = e.id;
        ev["timestamp"]   = (double)e.timestamp;
        ev["type"]        = e.typeString();
        ev["target_id"]   = e.targetId;
        ev["message"]     = e.message;
        ev["score_delta"] = e.scoreDelta;
        events.append(ev);
    }
    root["events"] = events;

    if (!screenshotPath.isEmpty())
        root["screenshot"] = screenshotPath;

    root["safety_note"] = "Virtual-only evaluation. No real-world targeting, weapon control, ballistic or damage model.";

    QJsonDocument doc(root);
    f.write(doc.toJson(QJsonDocument::Indented));
    f.close();
    return true;
}

bool MissionReport::exportCsv(const QString& path,
                               const MissionState& state,
                               const ScoreEvaluator& evaluator,
                               const EventTimeline& timeline) {
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QTextStream ts(&f);

    // 任务摘要
    ts << "Field,Value\n";
    ts << "mission_id,"       << state.missionName << "\n";
    ts << "mission_status,"   << state.statusString() << "\n";
    ts << "start_time,"       << state.startTime.toString(Qt::ISODate) << "\n";
    ts << "end_time,"         << state.endTime.toString(Qt::ISODate) << "\n";
    ts << "elapsed_sec,"      << state.elapsedSec << "\n";
    ts << "path_length_m,"    << state.pathLength << "\n";
    ts << "battery_pct,"      << state.batteryEstimate << "\n";
    ts << "targets_completed,"<< state.targetsCompleted << "\n";
    ts << "zones_completed,"  << state.zonesCompleted << "\n";
    ts << "events_triggered," << state.eventsTriggered << "\n";
    ts << "target_score,"     << evaluator.targetCompletionScore() << "\n";
    ts << "zone_score,"       << evaluator.zoneCoverageScore() << "\n";
    ts << "path_score,"       << evaluator.pathEfficiencyScore() << "\n";
    ts << "time_score,"       << evaluator.timeEfficiencyScore() << "\n";
    ts << "energy_score,"     << evaluator.energyEstimateScore() << "\n";
    ts << "event_bonus,"      << evaluator.eventBonus() << "\n";
    ts << "final_score,"      << evaluator.totalScore() << "\n";
    ts << "max_score,"        << evaluator.maxScore() << "\n";
    ts << "\n";

    // 事件列表
    ts << "id,timestamp,type,target_id,message,score_delta\n";
    for (const auto& e : timeline.events()) {
        ts << e.id << "," << e.timestamp << "," << e.typeString() << ","
           << e.targetId << ",\"" << e.message << "\"," << e.scoreDelta << "\n";
    }

    f.close();
    return true;
}
