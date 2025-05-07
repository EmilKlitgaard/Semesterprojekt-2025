#pragma once
#include<QApplication>

class GUI {
public:
    //set
    void setDifficulty(int value){
        if(!gameActive) {
            if (value < 300) {
                difficulty = 300;
            }
            if (value > 3000) {
                difficulty = 3000;
            }
            difficulty = value;
        }

        /*
        setoption name UCI_LimitStrength value true
        setoption name UCI_Elo value 1200 
        */

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

    void setVision() {
        
    }

    void getVision() {

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
    int difficulty = 300;

    Stockfish *stockfishRef = nullptr;

};

extern GUI gui;