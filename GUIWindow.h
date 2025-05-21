#pragma once

#include <QGroupBox>
#include <QFont>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QtCore/QTimer>
#include <QtGui/QImage>
#include <QGroupBox>
#include <QEventLoop>
#include <QMetaObject>
#include <QThread>
#include <opencv2/opencv.hpp>

#include "Vision.h"

class GUIWindow : public QMainWindow {
    Q_OBJECT

    public:
        void setTurnStatus();
        void setConnectionStatus();
        void updateDifficultyLabel(int val);

        int getSliderValue() const;
        
        string selectPawnPromotion();
        explicit GUIWindow(QWidget* parent = nullptr);
        
    signals:
        void startClicked();
        void resetClicked();
        void calibrateClicked();
        void difficultyChanged(int value);

    private slots:
        // void updateVision();
        void handleSliderChanged(int value);
        void handleStartClicked();
        void handleResetClicked();
        void handleCalibrateClicked();

    private:
        void updateWindow();
        Q_INVOKABLE QString selectPawnPromotionImpl();

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
        QPushButton calibrateTool;

        QLabel headerLabel;
        //QLabel* cvLabel;
        //QTimer cvTimer;
        //QImage cvFeed;

        QGroupBox* promotionBox = nullptr;
};

extern GUIWindow* window;