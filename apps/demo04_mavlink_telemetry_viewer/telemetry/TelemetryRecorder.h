#pragma once
#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QString>
#include "DroneTelemetryState.h"

// 遥测日志记录器
// 将遥测帧以 JSONL 格式写入 outputs/demo04_logs/ 目录
// 注意：不应提交真实飞行日志到 GitHub（已在 .gitignore 中排除 outputs/）
class TelemetryRecorder : public QObject {
    Q_OBJECT

public:
    explicit TelemetryRecorder(QObject* parent = nullptr);
    ~TelemetryRecorder() override;

    bool startRecording(const QString& outputDir, const QString& vehicleId);
    void stopRecording();
    bool isRecording() const { return m_recording; }
    QString currentLogPath() const { return m_logPath; }

public slots:
    void record(const DroneTelemetryState& state);

signals:
    void logSaved(const QString& path);
    void errorOccurred(const QString& error);

private:
    QFile*       m_file      = nullptr;
    QTextStream* m_stream    = nullptr;
    bool         m_recording = false;
    QString      m_logPath;
    int          m_frameCount = 0;
};
