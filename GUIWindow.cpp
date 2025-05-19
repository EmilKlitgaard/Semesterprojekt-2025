#include <opencv2/opencv.hpp>

#include "GUI.h"
#include "GUIWindow.h"
#include "Game.h"

Game game;

GUIWindow::GUIWindow(QWidget* parent)
    : QMainWindow(parent), central(this), mainLayout(&central), difficultyLabel(this), difficultySlider(Qt::Horizontal, this), cvLabel(new QLabel(this)), startGame("Start", this), resetGame("Reset", this) {
    resize(1000, 300);
    
    difficultySlider.setRange(300, 3000);
    difficultySlider.setValue(gui.getDifficulty());

    setTurnStatus();

    statusLayout.addWidget(&connectionStatus);
    statusLayout.addWidget(&turnStatus);
    mainLayout.addLayout(&statusLayout);

    difficultyLayout.addWidget(&difficultyLabel);
    difficultyLayout.addWidget(&difficultySlider);
    mainLayout.addLayout(&difficultyLayout);

    cvTimer.setInterval(30);
    mainLayout.addWidget(cvLabel);

    startStopLayout.addWidget(&startGame);
    startStopLayout.addWidget(&resetGame);
    mainLayout.addLayout(&startStopLayout);

    setCentralWidget(&central);

    if (!gui.getGameActive()) {
        gui.setDifficulty(getSliderValue());
    }

    connect(&cvTimer, &QTimer::timeout, this, &GUIWindow::updateVision);
    connect(&difficultySlider, &QSlider::valueChanged, this, &GUIWindow::handleSliderChanged);
    connect(&startGame, &QPushButton::clicked, this, &GUIWindow::handleStartClicked);
    connect(&resetGame, &QPushButton::clicked, this, &GUIWindow::handleResetClicked);
    
    windowUpdateTimer.setInterval(100); // Update every 100 ms
    connect(&windowUpdateTimer, &QTimer::timeout, this, &GUIWindow::updateWindow);
    windowUpdateTimer.start();
}

void GUIWindow::updateWindow() {
    setConnectionStatus();
    setTurnStatus();
}

int GUIWindow::getSliderValue() const { return difficultySlider.value(); }

void GUIWindow::updateVision() {
    // get Mat from GUI
    cv::Mat mat = gui.getVision ();
    if (mat.empty()) return;

    // convert BGR -> RGB
    cv::Mat rgb;
    cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);

    // Build QImage
    QImage img((uchar*)rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888);
    cvFeed = img.copy();
    cvLabel->setPixmap(QPixmap::fromImage(cvFeed));
}

void GUIWindow::setTurnStatus() {
    turnStatus.setText(gui.getTurn() == "Robot" ? "Turn: Robot" : "Turn: Player");
}

void GUIWindow::setConnectionStatus() {
    connectionStatus.setText(gui.getConnection() ? "Connected" : "Disconnected");
    connectionStatus.setStyleSheet(gui.getConnection() ? "color: green;" : "color: red;");
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
    if (!gui.getGameActive()) {
        gui.setGameActive(true);
        game.startGame();
        emit startClicked();
    }
}

void GUIWindow::handleResetClicked() {
    if (gui.getGameActive()) {
        gui.setGameActive(false);
        game.resetChessboard();
        emit stopClicked();
    }
}