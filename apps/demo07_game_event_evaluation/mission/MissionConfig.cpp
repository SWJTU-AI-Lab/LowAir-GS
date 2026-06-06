#include "MissionConfig.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QDir>

bool MissionConfig::loadFromFile(const QString& path, MissionConfig& cfg) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return false;
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();
    if (!doc.isObject()) return false;

    QJsonObject root = doc.object();
    cfg.scenarioName = root["scenario_name"].toString();
    cfg.missionId    = root["scenario_name"].toString();

    // 场景路径（相对于 JSON 文件目录）
    QFileInfo fi(path);
    QString dir = fi.absoluteDir().absolutePath();
    QJsonObject scene = root["scene"].toObject();
    cfg.meshPath     = scene["mesh_path"].toString();
    cfg.gaussianPath = scene["gaussian_path"].toString();

    QJsonObject replay = root["replay"].toObject();
    cfg.replayTrackPath = replay["track_path"].toString();
    cfg.replaySpeed     = (float)replay["speed"].toDouble(1.0);
    cfg.replayLoop      = replay["loop"].toBool(true);

    QJsonObject scoring = root["scoring"].toObject();
    cfg.maxScore = scoring["max_score"].toInt(100);
    QJsonObject rules = scoring["rules"].toObject();
    cfg.scoreTargetReached  = rules["target_reached"].toInt(20);
    cfg.scoreZoneCoverage   = rules["zone_coverage"].toInt(30);
    cfg.scorePathEfficiency = rules["path_efficiency"].toInt(20);
    cfg.scoreTimeEfficiency = rules["time_efficiency"].toInt(15);
    cfg.scoreEnergyEstimate = rules["energy_estimate"].toInt(15);

    // 虚拟目标
    cfg.targets.clear();
    for (const QJsonValue& tv : root["targets"].toArray()) {
        QJsonObject to = tv.toObject();
        VirtualTarget t;
        t.id         = to["id"].toString();
        t.name       = to["name"].toString();
        t.type       = to["type"].toString();
        QJsonArray pos = to["position"].toArray();
        if (pos.size() >= 3)
            t.position = {(float)pos[0].toDouble(), (float)pos[1].toDouble(), (float)pos[2].toDouble()};
        t.radius     = (float)to["radius"].toDouble(2.0);
        t.score      = to["score"].toInt(10);
        t.effectType = to["effect"].toString("glow");
        cfg.targets.push_back(t);
    }

    // 触发区域
    cfg.zones.clear();
    for (const QJsonValue& zv : root["targets"].toArray()) {
        QJsonObject zo = zv.toObject();
        if (zo["type"].toString() != "inspection_zone") continue;
        TriggerZone z;
        z.id         = zo["id"].toString();
        z.name       = zo["name"].toString();
        z.type       = zo["type"].toString();
        QJsonArray ctr = zo["center"].toArray();
        if (ctr.size() >= 3)
            z.center = {(float)ctr[0].toDouble(), (float)ctr[1].toDouble(), (float)ctr[2].toDouble()};
        QJsonArray sz = zo["size"].toArray();
        if (sz.size() >= 3)
            z.size = {(float)sz[0].toDouble(), (float)sz[1].toDouble(), (float)sz[2].toDouble()};
        z.score      = zo["score"].toInt(20);
        z.effectType = zo["effect"].toString("pulse");
        cfg.zones.push_back(z);
    }

    return true;
}
