#include "TelemetryRecorder.h"
#include <QDir>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonDocument>

TelemetryRecorder::TelemetryRecorder(QObject* parent) : QObject(parent) {}

TelemetryRecorder::~TelemetryRecorder() {
    stopRecording();
}

bool TelemetryRecorder::startRecording(const QString& outputDir, const QString& vehicleId) {
    if (m_recording) stopRecording();

    QDir dir(outputDir);
    if (!dir.exists()) dir.mkpath(".");

    QString ts   = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    m_logPath    = dir.filePath(QString("demo04_%1_%2.jsonl").arg(vehicleId).arg(ts));

    m_file   = new QFile(m_logPath, this);
    if (!m_file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit errorOccurred(QString("[Recorder] Cannot open log file: %1").arg(m_logPath));
        delete m_file;
        m_file = nullptr;
        return false;
    }
    m_stream     = new QTextStream(m_file);
    m_recording  = true;
    m_frameCount = 0;
    return true;
}

void TelemetryRecorder::stopRecording() {
    if (!m_recording) return;
    m_recording = false;
    if (m_stream) { m_stream->flush(); delete m_stream; m_stream = nullptr; }
    if (m_file)   { m_file->close();   delete m_file;   m_file   = nullptr; }
    emit logSaved(m_logPath);
}

void TelemetryRecorder::record(const DroneTelemetryState& s) {
    if (!m_recording || !m_stream) return;

    QJsonObject obj;
    obj["timestamp_ms"]   = s.timestampMs;
    obj["source"]         = s.sourceType;
    obj["vehicle_id"]     = s.vehicleId;
    obj["lat"]            = s.latitudeDeg;
    obj["lon"]            = s.longitudeDeg;
    obj["alt"]            = s.absoluteAltitudeM;
    obj["relative_alt"]   = s.relativeAltitudeM;
    obj["enu_east"]       = s.enuEastM;
    obj["enu_north"]      = s.enuNorthM;
    obj["enu_up"]         = s.enuUpM;
    obj["roll"]           = s.rollDeg;
    obj["pitch"]          = s.pitchDeg;
    obj["yaw"]            = s.yawDeg;
    obj["vn"]             = s.velocityNorthMps;
    obj["ve"]             = s.velocityEastMps;
    obj["vd"]             = s.velocityDownMps;
    obj["battery"]        = s.batteryPercent;
    obj["flight_mode"]    = s.flightMode;
    obj["armed"]          = s.armed;
    obj["gps_valid"]      = s.gpsValid;
    obj["connected"]      = s.connected;

    *m_stream << QJsonDocument(obj).toJson(QJsonDocument::Compact) << "\n";
    m_frameCount++;

    // 每 100 帧刷新一次
    if (m_frameCount % 100 == 0) m_stream->flush();
}
