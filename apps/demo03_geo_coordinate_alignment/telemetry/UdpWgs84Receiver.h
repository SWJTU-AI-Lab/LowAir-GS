#pragma once

// UdpWgs84Receiver.h
// UDP WGS84 遥测接收器
// 监听指定端口，解析 WGS84 JSON 格式无人机遥测数据

#include <QObject>
#include <QUdpSocket>
#include "GeoUavState.h"

class UdpWgs84Receiver : public QObject {
    Q_OBJECT

public:
    explicit UdpWgs84Receiver(QObject* parent = nullptr);
    ~UdpWgs84Receiver();

    bool startListening(quint16 port);
    void stopListening();
    bool isListening() const;
    quint16 port() const { return m_port; }

signals:
    void stateReceived(const GeoUavState& state);
    void errorOccurred(const QString& msg);
    void logMessage(const QString& msg);

private slots:
    void onReadyRead();

private:
    GeoUavState parseJson(const QByteArray& data, bool& ok);

    QUdpSocket* m_socket = nullptr;
    quint16     m_port   = 14581;
};
