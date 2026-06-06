#include "UdpWgs84Receiver.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QHostAddress>

UdpWgs84Receiver::UdpWgs84Receiver(QObject* parent)
    : QObject(parent) {
    m_socket = new QUdpSocket(this);
    connect(m_socket, &QUdpSocket::readyRead, this, &UdpWgs84Receiver::onReadyRead);
}

UdpWgs84Receiver::~UdpWgs84Receiver() {
    stopListening();
}

bool UdpWgs84Receiver::startListening(quint16 port) {
    m_port = port;
    if (m_socket->state() == QAbstractSocket::BoundState) {
        m_socket->close();
    }
    bool ok = m_socket->bind(QHostAddress::AnyIPv4, port);
    if (ok) {
        emit logMessage(QString("[UDP] Listening on port %1 (WGS84 format)").arg(port));
    } else {
        emit errorOccurred(QString("[UDP] Failed to bind port %1: %2")
                           .arg(port).arg(m_socket->errorString()));
    }
    return ok;
}

void UdpWgs84Receiver::stopListening() {
    if (m_socket->state() == QAbstractSocket::BoundState) {
        m_socket->close();
        emit logMessage("[UDP] Stopped listening");
    }
}

bool UdpWgs84Receiver::isListening() const {
    return m_socket->state() == QAbstractSocket::BoundState;
}

void UdpWgs84Receiver::onReadyRead() {
    while (m_socket->hasPendingDatagrams()) {
        QByteArray data;
        data.resize(static_cast<int>(m_socket->pendingDatagramSize()));
        m_socket->readDatagram(data.data(), data.size());

        bool ok = false;
        GeoUavState state = parseJson(data, ok);
        if (ok) {
            emit stateReceived(state);
        } else {
            emit logMessage(QString("[UDP] Parse error: %1").arg(QString(data.left(80))));
        }
    }
}

GeoUavState UdpWgs84Receiver::parseJson(const QByteArray& data, bool& ok) {
    GeoUavState state;
    ok = false;

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        return state;
    }

    QJsonObject obj = doc.object();

    // 检查 frame 字段
    if (obj.contains("frame") && obj["frame"].toString() != "WGS84") {
        emit logMessage("[UDP] Warning: frame is not WGS84, ignoring");
        return state;
    }

    state.uavId       = obj.value("uav_id").toString("unknown");
    state.timestampMs = static_cast<qint64>(obj.value("timestamp_ms").toDouble(0));
    state.frame       = obj.value("frame").toString("WGS84");
    state.lat         = obj.value("lat").toDouble(0.0);
    state.lon         = obj.value("lon").toDouble(0.0);
    state.alt         = obj.value("alt").toDouble(0.0);
    state.roll        = obj.value("roll").toDouble(0.0);
    state.pitch       = obj.value("pitch").toDouble(0.0);
    state.yaw         = obj.value("yaw").toDouble(0.0);
    state.battery     = obj.value("battery").toDouble(100.0);
    state.valid       = true;

    ok = true;
    return state;
}
