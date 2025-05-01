#pragma once
#include<QTcpSocket>
#include<QObject>


class SocketClient : public QObject {
    Q_OBJECT
public:
    explicit SocketClient(QObject* parent = nullptr);
    void connectToServer(const QString& host = "127.0.0.1", quint16 port = 5555);
    void sendMessage(const QString& msg);

signals:
    void messageReceived(const QString& msg);

private slots: 
    void readServer();

private:
    QTcpSocket* socket;
};