#include "FusionConfig.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

FusionConfig FusionConfig::defaultConfig() {
    FusionConfig cfg;
    cfg.sceneName       = "demo06_default";
    cfg.coordinateFrame = "LOCAL_SCENE";
    cfg.unit            = "meter";
    return cfg;
}

FusionConfig FusionConfig::loadFromFile(const QString &path, bool *ok) {
    FusionConfig cfg = defaultConfig();
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        if (ok) *ok = false;
        return cfg;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (!doc.isObject()) {
        if (ok) *ok = false;
        return cfg;
    }
    QJsonObject root = doc.object();
    if (root.contains("scene_name"))       cfg.sceneName       = root["scene_name"].toString().toStdString();
    if (root.contains("coordinate_frame")) cfg.coordinateFrame = root["coordinate_frame"].toString().toStdString();
    if (root.contains("unit"))             cfg.unit            = root["unit"].toString().toStdString();

    // Mesh 配置
    if (root.contains("mesh")) {
        QJsonObject m = root["mesh"].toObject();
        if (m.contains("path"))        cfg.meshPath        = m["path"].toString().toStdString();
        if (m.contains("enabled"))     cfg.meshEnabled     = m["enabled"].toBool();
        if (m.contains("opacity"))     cfg.meshOpacity     = (float)m["opacity"].toDouble();
        if (m.contains("render_mode")) cfg.meshRenderMode  = m["render_mode"].toString().toStdString();
    }
    // Gaussian 配置
    if (root.contains("gaussian")) {
        QJsonObject g = root["gaussian"].toObject();
        if (g.contains("path"))        cfg.gaussianPath        = g["path"].toString().toStdString();
        if (g.contains("enabled"))     cfg.gaussianEnabled     = g["enabled"].toBool();
        if (g.contains("opacity"))     cfg.gaussianOpacity     = (float)g["opacity"].toDouble();
        if (g.contains("render_mode")) cfg.gaussianRenderMode  = g["render_mode"].toString().toStdString();
    }
    // 对齐参数
    if (root.contains("alignment")) {
        QJsonObject a = root["alignment"].toObject();
        if (a.contains("translation")) {
            QJsonArray t = a["translation"].toArray();
            if (t.size() == 3) cfg.alignTranslation = {(float)t[0].toDouble(), (float)t[1].toDouble(), (float)t[2].toDouble()};
        }
        if (a.contains("rotation_deg")) {
            QJsonArray r = a["rotation_deg"].toArray();
            if (r.size() == 3) cfg.alignRotationDeg = {(float)r[0].toDouble(), (float)r[1].toDouble(), (float)r[2].toDouble()};
        }
        if (a.contains("scale")) cfg.alignScale = (float)a["scale"].toDouble();
    }
    // 显示设置
    if (root.contains("display")) {
        QJsonObject d = root["display"].toObject();
        if (d.contains("mode"))               cfg.displayMode          = d["mode"].toString().toStdString();
        if (d.contains("show_grid"))           cfg.showGrid             = d["show_grid"].toBool();
        if (d.contains("show_axis"))           cfg.showAxis             = d["show_axis"].toBool();
        if (d.contains("show_mesh_bbox"))      cfg.showMeshBbox         = d["show_mesh_bbox"].toBool();
        if (d.contains("show_gaussian_bbox"))  cfg.showGaussianBbox     = d["show_gaussian_bbox"].toBool();
        if (d.contains("show_fps"))            cfg.showFps              = d["show_fps"].toBool();
    }
    if (ok) *ok = true;
    return cfg;
}

bool FusionConfig::saveToFile(const QString &path) const {
    QJsonObject root;
    root["scene_name"]       = QString::fromStdString(sceneName);
    root["coordinate_frame"] = QString::fromStdString(coordinateFrame);
    root["unit"]             = QString::fromStdString(unit);

    QJsonObject m;
    m["path"]        = QString::fromStdString(meshPath);
    m["enabled"]     = meshEnabled;
    m["opacity"]     = meshOpacity;
    m["render_mode"] = QString::fromStdString(meshRenderMode);
    root["mesh"] = m;

    QJsonObject g;
    g["path"]        = QString::fromStdString(gaussianPath);
    g["enabled"]     = gaussianEnabled;
    g["opacity"]     = gaussianOpacity;
    g["render_mode"] = QString::fromStdString(gaussianRenderMode);
    root["gaussian"] = g;

    QJsonObject a;
    a["translation"]  = QJsonArray{alignTranslation.x(), alignTranslation.y(), alignTranslation.z()};
    a["rotation_deg"] = QJsonArray{alignRotationDeg.x(), alignRotationDeg.y(), alignRotationDeg.z()};
    a["scale"]        = alignScale;
    root["alignment"] = a;

    QJsonObject d;
    d["mode"]               = QString::fromStdString(displayMode);
    d["show_grid"]           = showGrid;
    d["show_axis"]           = showAxis;
    d["show_mesh_bbox"]      = showMeshBbox;
    d["show_gaussian_bbox"]  = showGaussianBbox;
    d["show_fps"]            = showFps;
    root["display"] = d;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) return false;
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();
    return true;
}
