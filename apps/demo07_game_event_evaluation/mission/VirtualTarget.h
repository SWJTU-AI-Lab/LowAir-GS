#pragma once
#include <QString>
#include <QVector3D>

// 虚拟任务目标（兴趣点/检查点/标注点）
// 注意：这里的"目标"仅表示虚拟任务目标、兴趣点或检查点，不表示真实打击目标
struct VirtualTarget {
    QString   id;
    QString   name;
    QString   type;         // interest_point / checkpoint / marker
    QVector3D position;
    float     radius   = 2.0f;
    int       score    = 10;
    QString   effectType;   // glow / pulse / label
    bool      triggered = false;
    bool      completed = false;
};
