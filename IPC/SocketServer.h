#pragma once
#include <QTcpServer>
#include <QTcpSocket>
#include <QString>

class SocketServer : public QObject {
    Q_OBJECT

public:
    explicit SocketServer(QObject* parent = nullptr);
    void startServer(quint16 port =5555);
    void sendMessage(const QString& msg);

signals:
    void messageReceived(const QString& msg);

private slots:
    void newConnection();
    void readClient();

private:
    QTcpServer* server;
    QTcpSocket* clientSocket = nullptr;
};