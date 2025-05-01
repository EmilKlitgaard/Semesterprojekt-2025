#include "SocketServer.h"

SocketServer::SocketServer(QObject* parent) : QObject(parent) {
    server = new QTcpServer(this);
    connect(server, &QTcpServer::newConnection, this, &SocketServer::newConnection);
} 

void SocketServer::startServer(quint16 port) {
    server->listen(QHostAddress::Any, port); // open connections
}

void SocketServer::newConnection() {
    clientSocket = server->nextPendingConnection(); // fetch connection
    connect(clientSocket, &QTcpSocket::readyRead, this, &SocketServer::readClient);
}

void SocketServer::readClient() {
    QByteArray data = clientSocket->readAll(); // read message
    emit messageReceived(QString::fromUtf8(data)); // forward message
}

void SocketServer::sendMessage(const QString& msg) {
    if (clientSocket && clientSocket->isOpen()) {
        clientSocket->write(msg.toUtf8()); // respond back to GUI
    }
}