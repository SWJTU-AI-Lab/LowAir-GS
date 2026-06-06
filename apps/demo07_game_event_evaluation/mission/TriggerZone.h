#pragma once
#include <QString>
#include <QVector3D>

// 任务触发区域（虚拟区域，用于教学任务事件触发）
struct TriggerZone {
    QString   id;
    QString   name;
    QString   type;         // inspection_zone / boundary / patrol_zone
    QVector3D center;
    QVector3D size = {5.0f, 5.0f, 4.0f};   // 半轴尺寸（x, y, z）
    int       score    = 20;
    QString   effectType;   // pulse / smoke_marker / ring_wave
    bool      triggered = false;
    bool      completed = false;

    // 判断点是否在区域内（AABB）
    bool contains(const QVector3D& pt) const {
        return std::abs(pt.x() - center.x()) <= size.x() * 0.5f &&
               std::abs(pt.y() - center.y()) <= size.y() * 0.5f &&
               std::abs(pt.z() - center.z()) <= size.z() * 0.5f;
    }
};
