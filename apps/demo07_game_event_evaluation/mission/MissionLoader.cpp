#include "MissionLoader.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

bool MissionLoader::loadTargets(const QString& path, std::vector<VirtualTarget>& targets) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return false;
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();
    QJsonArray arr = doc.isArray() ? doc.array() : doc.object()["targets"].toArray();
    targets.clear();
    for (const QJsonValue& v : arr) {
        QJsonObject o = v.toObject();
        VirtualTarget t;
        t.id         = o["id"].toString();
        t.name       = o["name"].toString();
        t.type       = o["type"].toString("interest_point");
        QJsonArray pos = o["position"].toArray();
        if (pos.size() >= 3)
            t.position = {(float)pos[0].toDouble(), (float)pos[1].toDouble(), (float)pos[2].toDouble()};
        t.radius     = (float)o["radius"].toDouble(2.0);
        t.score      = o["score"].toInt(10);
        t.effectType = o["effect"].toString("glow");
        targets.push_back(t);
    }
    return true;
}

bool MissionLoader::loadZones(const QString& path, std::vector<TriggerZone>& zones) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return false;
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();
    QJsonArray arr = doc.isArray() ? doc.array() : doc.object()["zones"].toArray();
    zones.clear();
    for (const QJsonValue& v : arr) {
        QJsonObject o = v.toObject();
        TriggerZone z;
        z.id         = o["id"].toString();
        z.name       = o["name"].toString();
        z.type       = o["type"].toString("inspection_zone");
        QJsonArray ctr = o["center"].toArray();
        if (ctr.size() >= 3)
            z.center = {(float)ctr[0].toDouble(), (float)ctr[1].toDouble(), (float)ctr[2].toDouble()};
        QJsonArray sz = o["size"].toArray();
        if (sz.size() >= 3)
            z.size = {(float)sz[0].toDouble(), (float)sz[1].toDouble(), (float)sz[2].toDouble()};
        z.score      = o["score"].toInt(20);
        z.effectType = o["effect"].toString("pulse");
        zones.push_back(z);
    }
    return true;
}
