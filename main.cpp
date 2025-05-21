#include <QApplication>
#include <iostream>

#include "Game.h"
#include "GUIWindow.h"

using namespace std;

GUIWindow* window = nullptr;

//   ==========   INITIALIZE GUI   ==========   //
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    GUIWindow mainWindow;
    window = &mainWindow;
    window->show();

    cout << "GUI initialized." << endl;
    return app.exec();
}