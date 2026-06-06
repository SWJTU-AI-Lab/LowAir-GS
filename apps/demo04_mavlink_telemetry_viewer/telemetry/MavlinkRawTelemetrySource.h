#pragma once
#include "TelemetrySource.h"
#include <QUdpSocket>
#include <QTimer>
#include <QDateTime>
#include <QHostAddress>
#include <cstdint>

// MAVLink Raw 遥测源（兜底实现）
// 仅解析只读消息：HEARTBEAT(#0), GLOBAL_POSITION_INT(#33),
//                 ATTITUDE(#30), SYS_STATUS(#1), BATTERY_STATUS(#147),
//                 VFR_HUD(#74)
// 安全边界：不发送任何飞控控制命令
class MavlinkRawTelemetrySource : public TelemetrySource {
    Q_OBJECT

public:
    explicit MavlinkRawTelemetrySource(QObject* parent = nullptr);
    ~MavlinkRawTelemetrySource() override;

    bool    start(const QString& connectionUrl) override;
    void    stop()                              override;
    bool    isRunning() const                   override;
    QString name()      const                   override { return "MAVLinkRaw"; }

private slots:
    void onReadyRead();
    void onHeartbeatTimeout();

private:
    // MAVLink v1 最小帧解析（无需外部 mavlink 头文件）
    void parseRawBytes(const QByteArray& data);
    void dispatchMessage(uint8_t msgId, const uint8_t* payload, uint8_t len);

    // 消息解析器（只读）
    void handleHeartbeat(const uint8_t* p);
    void handleGlobalPositionInt(const uint8_t* p);
    void handleAttitude(const uint8_t* p);
    void handleSysStatus(const uint8_t* p);
    void handleBatteryStatus(const uint8_t* p);
    void handleVfrHud(const uint8_t* p);

    // 小端序读取工具
    static int32_t  readI32(const uint8_t* p, int offset);
    static uint32_t readU32(const uint8_t* p, int offset);
    static int16_t  readI16(const uint8_t* p, int offset);
    static uint16_t readU16(const uint8_t* p, int offset);
    static float    readF32(const uint8_t* p, int offset);

    QUdpSocket*          m_socket      = nullptr;
    QTimer*              m_hbTimer     = nullptr;  // 心跳超时检测
    bool                 m_running     = false;
    quint16              m_port        = 14550;
    DroneTelemetryState  m_state;
    qint64               m_lastHbMs    = 0;
    int                  m_packetCount = 0;
    qint64               m_rateTimer   = 0;
    int                  m_rateCount   = 0;
    double               m_rateHz      = 0.0;

    // MAVLink v1 解析状态机
    enum class ParseState { IDLE, GOT_STX, GOT_LEN, GOT_SEQ, GOT_SYS, GOT_COMP, IN_PAYLOAD, GOT_CRC1 };
    ParseState m_parseState = ParseState::IDLE;
    uint8_t    m_msgLen     = 0;
    uint8_t    m_msgSeq     = 0;
    uint8_t    m_sysId      = 0;
    uint8_t    m_compId     = 0;
    uint8_t    m_msgId      = 0;
    QByteArray m_payload;
    uint8_t    m_crc1       = 0;
};
