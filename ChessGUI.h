#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
#include <QSlider>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToggleButton>
#include <QSpinBox>

class ChessGui : public QMainWindow {
    Q_OBJECT

public:
    ChessGui(QWidget *parent = nullptr);

signals:
    void gameStarted();
    void boardReset();
    void voiceCommandToggled(bool enabled);
    void playerFirstToggled(bool playerFirst);
    void speedChessToggled(bool enabled);
    void speedChessTimeChanged(int minutes);
    void difficultyChanged(int value);

private:
    // GUI Elements
    QLabel *turnStatusLabel;
    QLabel *voiceManualLabel;
    QLabel *playerRobotLabel;
    QLabel *speedChessLabel;
    QLabel *difficultyLabel;

    QToggleButton *voiceToggle;
    QToggleButton *playerFirstToggle;
    QToggleButton *speedChessToggle;
    
    QLineEdit *speedChessTime;
    QSlider *difficultySlider;
    
    QPushButton *startButton;
    QPushButton *resetButton;

    void setupUi();
    void createConnections();
    void updateTurnStatus(const QString &status);

private slots:
    void onSpeedChessToggled(bool enabled);
};