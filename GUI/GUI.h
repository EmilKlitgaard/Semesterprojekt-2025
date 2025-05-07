#pragma once
#include<QApplication>
#include "Stockfish.h"

class GUI {
public:
    //set
    void setDifficulty(int value){
        if(!gameActive) {
            if (value < 0) {
                difficulty = 1;
            }
            if (value > 20) {
                difficulty = 20;
            }
            difficulty = value;
        }

        /*
        setoption name UCI_LimitStrength value true
        setoption name UCI_Elo value 1200 
        */
        
        engine.sendCommand("setoption name UCI_Elo value " + difficulty);

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
    int difficulty = 300;

};

extern GUI gui;