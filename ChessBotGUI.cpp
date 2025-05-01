#include "ChessBotGUI.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QTime>

#include <ur_rtde/rtde_control_interface.h>
#include <ur_rtde/rtde_receive_interface.h>

ChessBotGUI::ChessBotGUI(QWidget* parent) : QMainWindow(parent) {
    QWidget* centralWidget = new QWidget(this); // central GUI container

    QVBoxLayout* layout = new QVBoxLayout(centralWidget); //  vertical layout stack

    statusLabel = new QLabel("ChessBot initialized", this); // dummy label

    layout->addWidget(statusLabel); // add label to layout

    difficultySlider = new QSlider(Qt::Horizontal, this);
    difficultySlider->setRange(0, 20); // Difficulty: "1-2 -> ignorant of tactics, 3-4 -> reasonable moves w/o tactics,..., 20 -> world class"
    difficultySlider->setValue(3); 
    difficultySlider->setTickPosition(QSlider::TicksBelow);
    difficultySlider->setTickInterval(1);

    layout->addWidget(difficultySlider);

    connect(difficultySlider, &QSlider::valueChanged, this, &ChessBotGUI::onSliderChanged);



    QTimer* statusTimer = new QTimer(this);
    connect(statusTimer, &QTimer::timeout, this, [=](){
        QString connection = rtde_control.isConnected() ? "CONNECTED" : "DISCONNECTED";
        QString turn = ;
        statusLabel-> setText("Robot: "+ connection + "Turn: " + turn);
    });

statusTimer->start(1000);

    centralWidget->setLayout(layout); // add layout to central GUI container

    setCentralWidget(centralWidget);

    setWindowTitle("Chess Bot interface"); 
    resize(400, 200);
}

ChessBotGUI::~ChessBotGUI() {}

void ChessBotGUI::onSliderChanged(int difficulty) {
    statusLabel->setText(QString(""))
}

