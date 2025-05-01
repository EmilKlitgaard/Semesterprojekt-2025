#include <QApplication>
#include "ChessBotGUI.h"
#include "SocketClient.h"

SocketClient* socketClient = nullptr;

int main (int argc, char *argv[]) {
    QApplication app(argc, argv);

    socketClient = new SocketClient;
    socketClient->connectToServer(); // connect to robot program

    ChessBotGUI window;
    window.show();

    return app.exec();
}