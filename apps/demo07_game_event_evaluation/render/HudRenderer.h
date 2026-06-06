#pragma once
#include <QPainter>
#include <QWidget>
#include <QString>

// HUD 叠加渲染器（使用 QPainter 在 OpenGL 视口上绘制文字）
class HudRenderer {
public:
    HudRenderer() = default;

    void render(QPainter& painter, int viewW, int viewH,
                int score, int maxScore,
                float elapsedSec, float battery,
                int events, const QString& missionStatus);
};
