#include "TrajectoryReplay.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

TrajectoryReplay::TrajectoryReplay(QObject* parent) : QObject(parent) {
    // 50ms 定时器（20 Hz 回放驱动）
    m_timer.setInterval(50);
    connect(&m_timer, &QTimer::timeout, this, &TrajectoryReplay::onTick);
}

TrajectoryReplay::~TrajectoryReplay() { m_timer.stop(); }

bool TrajectoryReplay::loadTrack(const QString& path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return false;

    m_frames.clear();
    while (!f.atEnd()) {
        QByteArray line = f.readLine().trimmed();
        if (line.isEmpty()) continue;
        QJsonDocument doc = QJsonDocument::fromJson(line);
        if (!doc.isObject()) continue;
        QJsonObject o = doc.object();
        ReplayFrame fr;
        fr.timestamp = (float)o["t"].toDouble();
        QJsonArray pos = o["pos"].toArray();
        if (pos.size() >= 3)
            fr.position = {(float)pos[0].toDouble(), (float)pos[1].toDouble(), (float)pos[2].toDouble()};
        fr.yawDeg   = (float)o["yaw"].toDouble();
        fr.pitchDeg = (float)o["pitch"].toDouble();
        fr.rollDeg  = (float)o["roll"].toDouble();
        fr.battery  = (float)o["battery"].toDouble(100.0);
        m_frames.push_back(fr);
    }
    f.close();
    m_frameIndex  = 0;
    m_currentTime = 0.0f;
    return !m_frames.empty();
}

float TrajectoryReplay::totalDuration() const {
    if (m_frames.empty()) return 0.0f;
    return m_frames.back().timestamp;
}

void TrajectoryReplay::start() {
    if (m_frames.empty()) return;
    m_playing = true;
    m_timer.start();
}

void TrajectoryReplay::pause() {
    m_playing = false;
    m_timer.stop();
}

void TrajectoryReplay::reset() {
    m_timer.stop();
    m_playing     = false;
    m_frameIndex  = 0;
    m_currentTime = 0.0f;
}

void TrajectoryReplay::onTick() {
    if (m_frames.empty()) return;

    // 推进时间
    m_currentTime += 0.05f * m_speed;

    // 找到当前时间对应的帧（线性搜索，帧数不多）
    while (m_frameIndex + 1 < (int)m_frames.size() &&
           m_frames[m_frameIndex + 1].timestamp <= m_currentTime)
        m_frameIndex++;

    if (m_frameIndex >= (int)m_frames.size() - 1) {
        // 到达末尾
        emit frameUpdated(m_frames.back());
        if (m_loop) {
            m_frameIndex  = 0;
            m_currentTime = 0.0f;
        } else {
            m_playing = false;
            m_timer.stop();
            emit replayFinished();
        }
        return;
    }

    // 线性插值
    const ReplayFrame& a = m_frames[m_frameIndex];
    const ReplayFrame& b = m_frames[m_frameIndex + 1];
    float dt = b.timestamp - a.timestamp;
    float alpha = (dt > 1e-6f) ? (m_currentTime - a.timestamp) / dt : 0.0f;
    alpha = std::max(0.0f, std::min(1.0f, alpha));

    ReplayFrame cur;
    cur.timestamp = m_currentTime;
    cur.position  = a.position + (b.position - a.position) * alpha;
    cur.yawDeg    = a.yawDeg   + (b.yawDeg   - a.yawDeg)   * alpha;
    cur.pitchDeg  = a.pitchDeg + (b.pitchDeg - a.pitchDeg) * alpha;
    cur.rollDeg   = a.rollDeg  + (b.rollDeg  - a.rollDeg)  * alpha;
    cur.battery   = a.battery  + (b.battery  - a.battery)  * alpha;

    emit frameUpdated(cur);
}
