#include "ChessBotGUI.h"
#include <QVBoxLayout>
#include <QWidget>
 
ChessBotGUI::ChessBotGUI(QWidget* parent) : QMainWindow(parent) {
    QWidget* centralWidget = new QWidget(this); // central GUI container

    QVBoxLayout* layout = new QVBoxLayout(centralWidget); //  vertical layout stack

    statusLabel = new QLabel("ChessBot initialized", this); // dummy label

    layout->addWidget(statusLabel); // add label to layout

    centralWidget->setLayout(layout); // add layout to central GUI container

    setCentralWidget(centralWidget);

    setWindowTitle("Chess Bot interface"); 
    resize(400, 200);
}

ChessBotGUI::~ChessBotGUI() {}