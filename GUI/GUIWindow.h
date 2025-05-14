#pragma once
#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QTimer>
#include <QImage>
#include "Vision.h"



class GUIWindow : public QMainWindow {
    Q_OBJECT

public:
    GUIWindow(QWidget* parent = nullptr);
    void setTurnStatus();
    void setConnectionStatus(bool ok = false);
    int getSliderValue() const;
    void updateDifficultyLabel(int val);
    
signals:
    void startClicked();
    void stopClicked();
    void difficultyChanged(int value);

private slots:
    void updateVision();
    void handleSliderChanged(int value);
    void handleStartClicked();
    void handleStopClicked();

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

    QLabel* cvLabel;
    QTimer cvTimer;
    QImage cvFeed;


};

extern GUIWindow window;