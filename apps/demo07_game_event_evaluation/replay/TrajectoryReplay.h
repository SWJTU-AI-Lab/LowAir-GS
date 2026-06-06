#pragma once
#include "ReplayFrame.h"
#include <QObject>
#include <QTimer>
#include <QString>
#include <vector>

// 轨迹回放器：从 JSONL 文件加载轨迹，按时间步驱动无人机位置
class TrajectoryReplay : public QObject {
    Q_OBJECT
public:
    explicit TrajectoryReplay(QObject* parent = nullptr);
    ~TrajectoryReplay();

    bool loadTrack(const QString& path);
    void start();
    void pause();
    void reset();
    bool isPlaying() const { return m_playing; }
    bool isLoaded()  const { return !m_frames.empty(); }

    float speed() const { return m_speed; }
    void  setSpeed(float s) { m_speed = s; }
    bool  loop()  const { return m_loop; }
    void  setLoop(bool l)  { m_loop = l; }

    float currentTime() const { return m_currentTime; }
    float totalDuration() const;
    int   frameCount()    const { return (int)m_frames.size(); }

    const std::vector<ReplayFrame>& frames() const { return m_frames; }

signals:
    void frameUpdated(const ReplayFrame& frame);
    void replayFinished();

private slots:
    void onTick();

private:
    std::vector<ReplayFrame> m_frames;
    int    m_frameIndex  = 0;
    float  m_currentTime = 0.0f;
    float  m_speed       = 1.0f;
    bool   m_loop        = true;
    bool   m_playing     = false;
    QTimer m_timer;
};
