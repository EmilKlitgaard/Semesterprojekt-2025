#pragma once
#include<QApplication>
#include<iostream>
#include<QMainWindow>
#include<QString>
#include<QVBoxLayout>
#include<QHBoxLayout>
#include<QSlider>
#include<QLabel>
#include<QPushButton>

class GUI {
public:
    //set
    void setDifficulty(int value = 3){
        if(!gameActive) {
            if (value < 0) {
                difficulty = 1;
            }
            if (value > 20) {
                difficulty = 20;
            }
            difficulty = value;
        }
    }
    void setGameActive(bool value) {
        gameActive = value;
    }
    void setTurn(bool value) {
        turn = value; // 1 = player, 0 = robot
    }
    void setConnection(bool value) {
        connection = value; // 1 = connected, 0 = disconnected
    }

    //get
    int  getDifficulty() { return difficulty; }
    bool getGameActive() { return gameActive; }
    bool getTurn() { return turn; }
    bool getConnection() { return connection; }    

private:
    // param
    bool turn = true;
    bool connection = false;
    bool gameActive = false;
    int difficulty = 3;

};
extern GUI gui;

/*

class GUIWindow : public QMainWindow {
    Q_OBJECT
public:
    GUIWindow(QWidget* parent = nullptr) : 
    QMainWindow(parent), 
    central(this), 
    mainLayout(&central), 
    connectionStatus(), 
    turnStatus(),
    difficultyLabel("Difficulty: ", this), 
    difficultySlider(Qt::Horizontal, this), 
    startGame("Start", this),
    stopGame("Stop", this)
    {
        
        difficultySlider.setRange(1,20);
        difficultySlider.setValue(3);

        setConnectionStatus(false);
        setTurnStatus();


        // line 1
        statusLayout.addWidget(&connectionStatus);
        statusLayout.addWidget(&turnStatus);
        mainLayout.addLayout(&statusLayout);
        // line 2
        difficultyLayout.addWidget(&difficultyLabel);
        difficultyLayout.addWidget(&difficultySlider);
        mainLayout.addLayout(&difficultyLayout);
        // line 3
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
    void setTurnStatus () {
        turnStatus.setText(gui.getTurn() ? "Turn: Player" : "Turn: Robot");
    }
    void setConnectionStatus (bool ok = false) {
        if (gui.getConnection()) {
            connectionStatus.setText("Connected");
        }
        else {
            connectionStatus.setText("Disconnected");
        }
        connectionStatus.setStyleSheet(ok ? "color: green;" : "color: red;");
    }

    int getSliderValue() const {
        return difficultySlider.value();
    }
    void handleSliderChanged (int value) {
        gui.setDifficulty(value);
        emit difficultyChanged(value);
    }

signals:
    void startClicked();
    void stopClicked();
    void difficultyChanged(int value);

private slots:
    void handleStartClicked() {
        gui.setGameActive(true);
        emit startClicked();
    }
    void handleStopClicked() {
        gui.setGameActive(false);
        emit stopClicked();
    }

private:
    QWidget central;
    QVBoxLayout mainLayout;
    QHBoxLayout statusLayout;
    QHBoxLayout difficultyLayout;
    QHBoxLayout startStopLayout;
    QLabel turnStatus;
    QLabel connectionStatus;
    QLabel difficultyLabel;
    QSlider difficultySlider;
    QPushButton startGame;
    QPushButton stopGame;

};
extern GUIWindow window;

*/