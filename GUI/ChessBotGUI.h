#include<iostream>
#include<vector>
#include<string>

#include<QMainWindow>
#include<QLabel>
#include<QSlider>
#include<QProcess>



class ChessBotGUI : public QMainWindow {
    Q_OBJECT

private:
    std::string status;
    QLabel* statusLabel;
    QSlider* difficultySlider;

private slots:
    void onSliderChanged (int difficulty) {

    }

public:
    ChessBotGUI(QWidget* parent = nullptr);
    ~ChessBotGUI();

    std::string statusIndicator() {
        
        
        return status;
    }

protected:
};

