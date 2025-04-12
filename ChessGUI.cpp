#include "ChessGui.h"
#include <QApplication>
#include <QStyle>

ChessGui::ChessGui(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Chess bot");
    setFixedSize(400, 400);
    
    setupUi();
    createConnections();
}

void ChessGui::setupUi() {
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    setCentralWidget(centralWidget);

    // Turn Status
    turnStatusLabel = new QLabel("TURN STATUS", this);
    turnStatusLabel->setAlignment(Qt::AlignCenter);
    turnStatusLabel->setStyleSheet(
        "QLabel { background-color: white; padding: 10px; "
        "border: 1px solid black; margin-bottom: 10px; }"
    );
    mainLayout->addWidget(turnStatusLabel);

    // Voice/Manual Toggle
    QHBoxLayout *voiceLayout = new QHBoxLayout();
    voiceManualLabel = new QLabel("Voice / manual:", this);
    voiceToggle = new QToggleButton(this);
    voiceLayout->addWidget(voiceManualLabel);
    voiceLayout->addWidget(voiceToggle);
    mainLayout->addLayout(voiceLayout);

    // Player/Robot First Toggle
    QHBoxLayout *playerLayout = new QHBoxLayout();
    playerRobotLabel = new QLabel("Player / Robot first:", this);
    playerFirstToggle = new QToggleButton(this);
    playerLayout->addWidget(playerRobotLabel);
    playerLayout->addWidget(playerFirstToggle);
    mainLayout->addLayout(playerLayout);

    // Speed Chess Section
    QHBoxLayout *speedChessLayout = new QHBoxLayout();
    speedChessLabel = new QLabel("Speed chess:", this);
    speedChessTime = new QLineEdit("10 min", this);
    speedChessTime->setFixedWidth(60);
    speedChessToggle = new QToggleButton(this);
    speedChessLayout->addWidget(speedChessLabel);
    speedChessLayout->addWidget(speedChessTime);
    speedChessLayout->addWidget(speedChessToggle);
    mainLayout->addLayout(speedChessLayout);

    // Difficulty Slider
    QHBoxLayout *difficultyLayout = new QHBoxLayout();
    difficultyLabel = new QLabel("Difficulty:", this);
    difficultySlider = new QSlider(Qt::Horizontal, this);
    difficultySlider->setStyleSheet(
        "QSlider::groove:horizontal {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "                                stop:0 #98FB98, stop:1 #FFB6C1);"
        "    height: 20px;"
        "}"
        "QSlider::handle:horizontal {"
        "    background: white;"
        "    width: 40px;"
        "    margin: -5px 0;"
        "}"
    );
    difficultyLayout->addWidget(difficultyLabel);
    difficultyLayout->addWidget(difficultySlider);
    mainLayout->addLayout(difficultyLayout);

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    startButton = new QPushButton("START", this);
    resetButton = new QPushButton("RESET BOARD", this);
    startButton->setStyleSheet("QPushButton { background-color: #ADD8E6; padding: 10px; }");
    resetButton->setStyleSheet("QPushButton { background-color: #F0F0F0; padding: 10px; }");
    buttonLayout->addWidget(startButton);
    buttonLayout->addWidget(resetButton);
    mainLayout->addLayout(buttonLayout);

    // Add some stretch at the bottom
    mainLayout->addStretch();
}

void ChessGui::createConnections() {
    connect(startButton, &QPushButton::clicked, this, &ChessGui::gameStarted);
    connect(resetButton, &QPushButton::clicked, this, &ChessGui::boardReset);
    connect(voiceToggle, &QToggleButton::toggled, this, &ChessGui::voiceCommandToggled);
    connect(playerFirstToggle, &QToggleButton::toggled, this, &ChessGui::playerFirstToggled);
    connect(speedChessToggle, &QToggleButton::toggled, this, &ChessGui::onSpeedChessToggled);
    connect(difficultySlider, &QSlider::valueChanged, this, &ChessGui::difficultyChanged);
}

void ChessGui::onSpeedChessToggled(bool enabled) {
    speedChessTime->setEnabled(enabled);
    emit speedChessToggled(enabled);
    if (enabled) {
        bool ok;
        int minutes = speedChessTime->text().split(" ")[0].toInt(&ok);
        if (ok) {
            emit speedChessTimeChanged(minutes);
        }
    }
}

void ChessGui::updateTurnStatus(const QString &status) {
    turnStatusLabel->setText(status);
}