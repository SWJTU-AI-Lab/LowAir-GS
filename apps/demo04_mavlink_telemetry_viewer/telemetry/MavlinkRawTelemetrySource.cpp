#include "MavlinkRawTelemetrySource.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <cstring>
#include <cmath>

// MAVLink v1 STX 字节
static constexpr uint8_t MAVLINK_STX = 0xFE;

// MAVLink 消息 ID（只读）
static constexpr uint8_t MSG_HEARTBEAT           = 0;
static constexpr uint8_t MSG_SYS_STATUS          = 1;
static constexpr uint8_t MSG_ATTITUDE            = 30;
static constexpr uint8_t MSG_GLOBAL_POSITION_INT = 33;
static constexpr uint8_t MSG_VFR_HUD             = 74;
static constexpr uint8_t MSG_BATTERY_STATUS      = 147;

// 飞行模式映射（PX4 custom_mode 简化版）
static QString flightModeFromCustomMode(uint32_t customMode) {
    // PX4 主模式（高16位）
    uint8_t mainMode = (customMode >> 16) & 0xFF;
    switch (mainMode) {
        case 1:  return "MANUAL";
        case 2:  return "ALTCTL";
        case 3:  return "POSCTL";
        case 4:  return "AUTO";
        case 5:  return "ACRO";
        case 6:  return "OFFBOARD";
        case 7:  return "STABILIZED";
        case 8:  return "RATTITUDE";
        default: return QString("MODE_%1").arg(mainMode);
    }
}

MavlinkRawTelemetrySource::MavlinkRawTelemetrySource(QObject* parent)
    : TelemetrySource(parent)
{
    m_socket  = new QUdpSocket(this);
    m_hbTimer = new QTimer(this);
    m_hbTimer->setInterval(3000);  // 3秒无心跳则断线
    connect(m_socket,  &QUdpSocket::readyRead, this, &MavlinkRawTelemetrySource::onReadyRead);
    connect(m_hbTimer, &QTimer::timeout,       this, &MavlinkRawTelemetrySource::onHeartbeatTimeout);
}

MavlinkRawTelemetrySource::~MavlinkRawTelemetrySource() {
    stop();
}

bool MavlinkRawTelemetrySource::start(const QString& connectionUrl) {
    // 解析 udp://:PORT 格式
    quint16 port = 14550;
    if (connectionUrl.startsWith("udp://")) {
        QString portStr = connectionUrl.section(':', -1);
        bool ok = false;
        quint16 p = portStr.toUShort(&ok);
        if (ok && p > 0) port = p;
    }
    m_port = port;

    if (m_socket->state() == QAbstractSocket::BoundState) {
        m_socket->close();
    }
    bool ok = m_socket->bind(QHostAddress::AnyIPv4, port);
    if (ok) {
        m_running = true;
        m_state.connected = false;
        m_state.sourceType = "MAVLinkRaw";
        m_hbTimer->start();
        m_rateTimer = QDateTime::currentMSecsSinceEpoch();
        emit logMessage(QString("[MAVLink] Listening on UDP port %1 (read-only)").arg(port));
        emit connectionStatusChanged(false, QString("Waiting for MAVLink heartbeat on :%1...").arg(port));
    } else {
        emit errorOccurred(QString("[MAVLink] Failed to bind port %1: %2").arg(port).arg(m_socket->errorString()));
    }
    return ok;
}

void MavlinkRawTelemetrySource::stop() {
    m_hbTimer->stop();
    if (m_socket->state() == QAbstractSocket::BoundState) {
        m_socket->close();
    }
    m_running = false;
    if (m_state.connected) {
        m_state.connected = false;
        emit connectionStatusChanged(false, "[MAVLink] Disconnected");
    }
    emit logMessage("[MAVLink] Stopped");
}

bool MavlinkRawTelemetrySource::isRunning() const {
    return m_running;
}

void MavlinkRawTelemetrySource::onReadyRead() {
    while (m_socket->hasPendingDatagrams()) {
        QByteArray data;
        data.resize(static_cast<int>(m_socket->pendingDatagramSize()));
        m_socket->readDatagram(data.data(), data.size());
        parseRawBytes(data);
    }
}

void MavlinkRawTelemetrySource::onHeartbeatTimeout() {
    if (m_state.connected) {
        m_state.connected = false;
        emit connectionStatusChanged(false, "[MAVLink] Heartbeat timeout - connection lost");
        emit logMessage("[MAVLink] Heartbeat timeout");
    }
}

// MAVLink v1 帧格式：
// STX(0xFE) | LEN | SEQ | SYS | COMP | MSGID | PAYLOAD[LEN] | CRC_LO | CRC_HI
void MavlinkRawTelemetrySource::parseRawBytes(const QByteArray& data) {
    for (int i = 0; i < data.size(); ++i) {
        uint8_t byte = static_cast<uint8_t>(data[i]);
        switch (m_parseState) {
            case ParseState::IDLE:
                if (byte == MAVLINK_STX) m_parseState = ParseState::GOT_STX;
                break;
            case ParseState::GOT_STX:
                m_msgLen = byte;
                m_payload.clear();
                m_payload.reserve(m_msgLen);
                m_parseState = ParseState::GOT_LEN;
                break;
            case ParseState::GOT_LEN:
                m_msgSeq = byte;
                m_parseState = ParseState::GOT_SEQ;
                break;
            case ParseState::GOT_SEQ:
                m_sysId = byte;
                m_parseState = ParseState::GOT_SYS;
                break;
            case ParseState::GOT_SYS:
                m_compId = byte;
                m_parseState = ParseState::GOT_COMP;
                break;
            case ParseState::GOT_COMP:
                m_msgId = byte;
                m_parseState = (m_msgLen > 0) ? ParseState::IN_PAYLOAD : ParseState::GOT_CRC1;
                break;
            case ParseState::IN_PAYLOAD:
                m_payload.append(static_cast<char>(byte));
                if (m_payload.size() >= m_msgLen) {
                    m_parseState = ParseState::GOT_CRC1;
                }
                break;
            case ParseState::GOT_CRC1:
                m_crc1 = byte;
                m_parseState = ParseState::IDLE;
                // 跳过 CRC 校验，直接分发（演示用途）
                dispatchMessage(m_msgId,
                                reinterpret_cast<const uint8_t*>(m_payload.constData()),
                                static_cast<uint8_t>(m_payload.size()));
                break;
        }
    }
}

void MavlinkRawTelemetrySource::dispatchMessage(uint8_t msgId, const uint8_t* payload, uint8_t len) {
    m_packetCount++;
    m_rateCount++;

    // 计算遥测频率（每秒更新一次）
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (now - m_rateTimer >= 1000) {
        m_rateHz    = m_rateCount * 1000.0 / (now - m_rateTimer);
        m_rateCount = 0;
        m_rateTimer = now;
    }

    m_state.timestampMs = now;

    switch (msgId) {
        case MSG_HEARTBEAT:           if (len >= 9)  handleHeartbeat(payload);          break;
        case MSG_SYS_STATUS:          if (len >= 31) handleSysStatus(payload);           break;
        case MSG_ATTITUDE:            if (len >= 28) handleAttitude(payload);            break;
        case MSG_GLOBAL_POSITION_INT: if (len >= 28) handleGlobalPositionInt(payload);   break;
        case MSG_VFR_HUD:             if (len >= 20) handleVfrHud(payload);              break;
        case MSG_BATTERY_STATUS:      if (len >= 36) handleBatteryStatus(payload);       break;
        default: break;
    }
}

// HEARTBEAT 消息（ID=0）
// [0-3]: custom_mode(u32), [4]: type(u8), [5]: autopilot(u8),
// [6]: base_mode(u8), [7]: system_status(u8), [8]: mavlink_version(u8)
void MavlinkRawTelemetrySource::handleHeartbeat(const uint8_t* p) {
    uint32_t customMode = readU32(p, 0);
    uint8_t  baseMode   = p[6];

    m_state.armed      = (baseMode & 0x80) != 0;
    m_state.flightMode = flightModeFromCustomMode(customMode);
    m_state.vehicleId  = "mavlink_uav_01";

    m_lastHbMs = QDateTime::currentMSecsSinceEpoch();
    m_hbTimer->start();  // 重置超时计时器

    if (!m_state.connected) {
        m_state.connected = true;
        emit connectionStatusChanged(true, QString("[MAVLink] Connected (sysId=%1, mode=%2)")
                                     .arg(m_sysId).arg(m_state.flightMode));
        emit logMessage(QString("[MAVLink] First heartbeat received from sysId=%1").arg(m_sysId));
    }
}

// GLOBAL_POSITION_INT 消息（ID=33）
// [0-3]: time_boot_ms(u32), [4-7]: lat(i32, 1e-7 deg), [8-11]: lon(i32, 1e-7 deg),
// [12-15]: alt(i32, mm), [16-19]: relative_alt(i32, mm),
// [20-21]: vx(i16, cm/s), [22-23]: vy(i16, cm/s), [24-25]: vz(i16, cm/s),
// [26-27]: hdg(u16, cdeg)
void MavlinkRawTelemetrySource::handleGlobalPositionInt(const uint8_t* p) {
    m_state.latitudeDeg       = readI32(p, 4)  * 1e-7;
    m_state.longitudeDeg      = readI32(p, 8)  * 1e-7;
    m_state.absoluteAltitudeM = readI32(p, 12) * 1e-3;
    m_state.relativeAltitudeM = readI32(p, 16) * 1e-3;
    m_state.velocityNorthMps  = readI16(p, 20) * 0.01;
    m_state.velocityEastMps   = readI16(p, 22) * 0.01;
    m_state.velocityDownMps   = readI16(p, 24) * 0.01;

    // GPS 有效性：纬度非零即认为有效
    m_state.gpsValid = (std::abs(m_state.latitudeDeg) > 1e-6);

    emit telemetryUpdated(m_state);
}

// ATTITUDE 消息（ID=30）
// [0-3]: time_boot_ms(u32), [4-7]: roll(f32,rad), [8-11]: pitch(f32,rad),
// [12-15]: yaw(f32,rad), [16-19]: rollspeed, [20-23]: pitchspeed, [24-27]: yawspeed
void MavlinkRawTelemetrySource::handleAttitude(const uint8_t* p) {
    static constexpr double RAD2DEG = 180.0 / M_PI;
    m_state.rollDeg  = readF32(p, 4)  * RAD2DEG;
    m_state.pitchDeg = readF32(p, 8)  * RAD2DEG;
    m_state.yawDeg   = readF32(p, 12) * RAD2DEG;
}

// SYS_STATUS 消息（ID=1）
// [14-15]: battery_voltage(u16, mV), [16-17]: battery_current(i16, cA),
// [30]: battery_remaining(i8, %)
void MavlinkRawTelemetrySource::handleSysStatus(const uint8_t* p) {
    int8_t remaining = static_cast<int8_t>(p[30]);
    if (remaining >= 0) {
        m_state.batteryPercent = static_cast<double>(remaining);
    }
}

// BATTERY_STATUS 消息（ID=147）
// [28]: battery_remaining(i8, %)
void MavlinkRawTelemetrySource::handleBatteryStatus(const uint8_t* p) {
    int8_t remaining = static_cast<int8_t>(p[28]);
    if (remaining >= 0) {
        m_state.batteryPercent = static_cast<double>(remaining);
    }
}

// VFR_HUD 消息（ID=74）
// [0-3]: airspeed(f32), [4-7]: groundspeed(f32), [8-11]: alt(f32),
// [12-15]: climb(f32), [16-17]: heading(i16, deg), [18-19]: throttle(u16, %)
void MavlinkRawTelemetrySource::handleVfrHud(const uint8_t* p) {
    // 可用于补充高度信息
    (void)p;
}

// 小端序读取工具
int32_t MavlinkRawTelemetrySource::readI32(const uint8_t* p, int offset) {
    int32_t v;
    memcpy(&v, p + offset, 4);
    return v;
}
uint32_t MavlinkRawTelemetrySource::readU32(const uint8_t* p, int offset) {
    uint32_t v;
    memcpy(&v, p + offset, 4);
    return v;
}
int16_t MavlinkRawTelemetrySource::readI16(const uint8_t* p, int offset) {
    int16_t v;
    memcpy(&v, p + offset, 2);
    return v;
}
uint16_t MavlinkRawTelemetrySource::readU16(const uint8_t* p, int offset) {
    uint16_t v;
    memcpy(&v, p + offset, 2);
    return v;
}
float MavlinkRawTelemetrySource::readF32(const uint8_t* p, int offset) {
    float v;
    memcpy(&v, p + offset, 4);
    return v;
}
