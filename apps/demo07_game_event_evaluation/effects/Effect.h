#pragma once
#include <QVector3D>
#include <QVector4D>
#include <QString>

// 特效类型
enum class EffectType {
    Glow,       // 光晕（目标点到达）
    Pulse,      // 脉冲环（区域进入）
    Particle,   // 粒子爆发（任务完成）
    None
};

// 特效实例
struct Effect {
    EffectType type      = EffectType::None;
    QVector3D  position;
    QVector4D  color     = {1.0f, 0.8f, 0.2f, 1.0f};  // RGBA
    float      radius    = 2.0f;
    float      lifetime  = 2.0f;   // 总生命周期（秒）
    float      elapsed   = 0.0f;   // 已经过时间（秒）
    bool       active    = true;

    float alpha() const {
        float t = elapsed / lifetime;
        return active ? std::max(0.0f, 1.0f - t) : 0.0f;
    }

    static EffectType fromString(const QString& s) {
        if (s == "glow")     return EffectType::Glow;
        if (s == "pulse")    return EffectType::Pulse;
        if (s == "particle") return EffectType::Particle;
        return EffectType::Glow;
    }
};
