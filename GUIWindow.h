#pragma once

#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSlider>
#include <QtWidgets/QPushButton>
#include <QtCore/QTimer>
#include <QtGui/QImage>
#include <opencv2/opencv.hpp>

#include "Vision.h"

class GUIWindow : public QMainWindow {
    Q_OBJECT

    public:
        GUIWindow(QWidget* parent = nullptr);
        void setTurnStatus();
        void setConnectionStatus();
        int getSliderValue() const;
        void updateDifficultyLabel(int val);
        
    signals:
        void startClicked();
        void resetClicked();
        void difficultyChanged(int value);

    private slots:
        void updateVision();
        void handleSliderChanged(int value);
        void handleStartClicked();
        void handleResetClicked();

    private:
        void updateWindow();
        QTimer windowUpdateTimer;

        QWidget central;

        QVBoxLayout mainLayout;

        QHBoxLayout statusLayout;
        QHBoxLayout difficultyLayout;
        QHBoxLayout startresetLayout;

        QLabel turnStatus;
        QLabel connectionStatus;

        QLabel difficultyLabel;
        QSlider difficultySlider;

        QPushButton startGame;
        QPushButton resetGame;

        QLabel* cvLabel;
        QTimer cvTimer;
        QImage cvFeed;
};

extern GUIWindow window;