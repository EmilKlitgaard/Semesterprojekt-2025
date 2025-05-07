#include "GUIWindow.h"
#include "GUI.h" // for access to `gui`

GUIWindow::GUIWindow(QWidget* parent)
    : QMainWindow(parent), central(this), mainLayout(&central), difficultyLabel(this), difficultySlider(Qt::Horizontal, this), startGame("Start", this), stopGame("Stop", this)
{
    difficultySlider.setRange(300, 3000);
    difficultySlider.setValue(gui.getDifficulty());

    setConnectionStatus(false);
    setTurnStatus();

    statusLayout.addWidget(&connectionStatus);
    statusLayout.addWidget(&turnStatus);
    mainLayout.addLayout(&statusLayout);

    difficultyLayout.addWidget(&difficultyLabel);
    difficultyLayout.addWidget(&difficultySlider);
    mainLayout.addLayout(&difficultyLayout);

    startStopLayout.addWidget(&startGame);
    startStopLayout.addWidget(&stopGame);
    mainLayout.addLayout(&startStopLayout);

    setCentralWidget(&central);

    if (!gui.getGameActive()) {
        gui.setDifficulty(getSliderValue());
    }

    connect(&difficultySlider, &QSlider::valueChanged, this, &GUIWindow::handleSliderChanged);
    connect(&startGame, &QPushButton::clicked, this, &GUIWindow::handleStartClicked);
    connect(&stopGame, &QPushButton::clicked, this, &GUIWindow::handleStopClicked);
}

void GUIWindow::setTurnStatus() {
    turnStatus.setText(gui.getTurn() ? "Turn: Player" : "Turn: Robot");
}

void GUIWindow::setConnectionStatus(bool ok) {
    connectionStatus.setText(gui.getConnection() ? "Connected" : "Disconnected");
    connectionStatus.setStyleSheet(ok ? "color: green;" : "color: red;");
}

int GUIWindow::getSliderValue() const {
    return difficultySlider.value();
}

void GUIWindow::updateDifficultyLabel(int value) {
    difficultyLabel.setText(QString("Difficulty: %1").arg(value));
}

void GUIWindow::handleSliderChanged(int value) {
    gui.setDifficulty(value);
    updateDifficultyLabel(gui.getDifficulty());
    emit difficultyChanged(value);
}

void GUIWindow::handleStartClicked() {
    gui.setGameActive(true);
    emit startClicked();
}

void GUIWindow::handleStopClicked() {
    gui.setGameActive(false);
    emit stopClicked();
}