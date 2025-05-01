#include "SocketClient.h"

SocketClient::SocketClient(QObject* parent) : QObject(parent) {
    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::readyRead, this, &SocketClient::readServer);
}

void SocketClient::connectToServer(const QString& host, quint16 port) {
    socket->connectToHost(host, port); // connect to robot
}

void SocketClient::readServer() {
    QByteArray data = socket->readAll();
    emit messageReceived(QString::fromUtf8(data)); // message from robot
}

void SocketClient::sendMessage(const QString& msg) {
    if (socket->isOpen()) {
        socket->write(msg.toUtf8()); // send message to robot
    }
}



