#pragma once
#include <QElapsedTimer>
#include <QVector>

// FPS 计数器：滑动窗口平均帧率统计
class FpsCounter {
public:
    FpsCounter(int windowSize = 60);
    void tick();
    float fps() const;
    float frameTimeMs() const;
    void reset();
private:
    QElapsedTimer m_timer;
    QVector<qint64> m_frameTimes; // 每帧耗时（微秒）
    int m_windowSize;
    int m_idx = 0;
    bool m_started = false;
    qint64 m_lastTime = 0;
};
