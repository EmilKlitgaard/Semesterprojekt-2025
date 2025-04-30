#include<iostream>
#include<vector>
#include <QMainWindow>
#include <QLabel>



class ChessBotGUI : public QMainWindow {
    Q_OBJECT
private:
    QLabel* statusLabel;
public:
    ChessBotGUI(QWidget* parent = nullptr);
    ~ChessBotGUI();
protected:
};

