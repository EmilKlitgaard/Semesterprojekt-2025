#include <QtWidgets/QApplication>
#include <iostream>

#include "Game.h"
#include "GUIWindow.h"

using namespace std;

/*============================================================
            		    MAIN START
============================================================*/
int main() {
    //   ==========   INITIALIZE GUI   ==========   //
    int argc = 0;
    char *argv[] = {nullptr};
    QApplication app(argc, argv);

    GUIWindow window;
    window.show();
    cout << "GUI initialized." << endl;
    return app.exec();
}