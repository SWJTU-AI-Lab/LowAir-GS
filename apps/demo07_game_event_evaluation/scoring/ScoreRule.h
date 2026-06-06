#pragma once
#include <QString>

// 单条评分规则
struct ScoreRule {
    QString name;
    int     maxPoints = 0;
    QString description;
};
