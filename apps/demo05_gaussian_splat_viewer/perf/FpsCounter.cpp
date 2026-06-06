#include "FpsCounter.h"

FpsCounter::FpsCounter(int windowSize) : m_windowSize(windowSize) {
    m_frameTimes.resize(windowSize, 16667); // 默认 60fps
    m_timer.start();
}

void FpsCounter::tick() {
    qint64 now = m_timer.nsecsElapsed() / 1000; // 微秒
    if (m_started) {
        qint64 dt = now - m_lastTime;
        if (dt > 0) {
            m_frameTimes[m_idx % m_windowSize] = dt;
            ++m_idx;
        }
    } else {
        m_started = true;
    }
    m_lastTime = now;
}

float FpsCounter::fps() const {
    qint64 sum = 0;
    for (auto t : m_frameTimes) sum += t;
    float avgUs = (float)sum / m_windowSize;
    return avgUs > 0 ? 1e6f / avgUs : 0.0f;
}

float FpsCounter::frameTimeMs() const {
    return fps() > 0 ? 1000.0f / fps() : 0.0f;
}

void FpsCounter::reset() {
    m_frameTimes.fill(16667);
    m_idx = 0;
    m_started = false;
    m_timer.restart();
}
