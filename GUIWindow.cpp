#include "GUI.h"
#include "GUIWindow.h"
#include "Game.h"

GUIWindow::GUIWindow(QWidget* parent)
    : QMainWindow(parent), central(this), mainLayout(&central), headerLabel("Chess Robot", this), difficultyLabel(this), difficultySlider(Qt::Horizontal, this), startGame("Start", this), resetGame("Reset", this), calibrateTool("Calibrate Tool", this) { // cvLabel(new QLabel(this)),
    // Window setup
    setWindowTitle("Chess Robot Control");
    resize(1000, 400);

    setStyleSheet("background-color: #ffffff;");

    // Header styling
    QFont headerFont;
    headerFont.setPointSize(24);
    headerFont.setBold(true);
    headerLabel.setFont(headerFont);
    headerLabel.setAlignment(Qt::AlignCenter);
    mainLayout.addWidget(&headerLabel);

    // Common box style
    const QString boxStyle =
        "QGroupBox {"
        "  border: 1px solid gray;"
        "  border-radius: 10px;"
        "  padding: 10px;"
        "  margin-top: 1ex;"
        "}";

    // ===== Status Box =====
    QGroupBox* statusBox = new QGroupBox("Status", this);
    statusBox->setStyleSheet(boxStyle);
    QHBoxLayout* statusLayoutBox = new QHBoxLayout(statusBox);
    statusLayoutBox->setSpacing(20);

    connectionStatus.setTextInteractionFlags(Qt::NoTextInteraction);
    turnStatus.setTextInteractionFlags(Qt::NoTextInteraction);

    statusLayoutBox->addWidget(&connectionStatus);
    statusLayoutBox->addWidget(&turnStatus);
    mainLayout.addWidget(statusBox);

    // ===== Difficulty Box =====
    QGroupBox* difficultyBox = new QGroupBox("Difficulty", this);
    difficultyBox->setStyleSheet(boxStyle);
    QHBoxLayout* diffLayoutBox = new QHBoxLayout(difficultyBox);
    difficultySlider.setRange(300, 3000);
    difficultySlider.setValue(gui.getDifficulty());
    updateDifficultyLabel(gui.getDifficulty());
    diffLayoutBox->addWidget(&difficultyLabel);
    diffLayoutBox->addWidget(&difficultySlider);
    mainLayout.addWidget(difficultyBox);

    // ===== Controls Box =====
    QGroupBox* controlsBox = new QGroupBox("Controls", this);
    controlsBox->setStyleSheet(boxStyle);
    QHBoxLayout* ctrlLayoutBox = new QHBoxLayout(controlsBox);
    ctrlLayoutBox->addWidget(&startGame);
    ctrlLayoutBox->addWidget(&resetGame);
    ctrlLayoutBox->addWidget(&calibrateTool);
    mainLayout.addWidget(controlsBox);

    // Camera Feed
    // cvTimer.setInterval(30);
    // mainLayout.addWidget(cvLabel);

    setCentralWidget(&central);
    
    // Connections
    connect(&difficultySlider, &QSlider::valueChanged, this, &GUIWindow::handleSliderChanged);
    connect(&startGame, &QPushButton::clicked, this, &GUIWindow::handleStartClicked);
    connect(&resetGame, &QPushButton::clicked, this, &GUIWindow::handleResetClicked);
    connect(&calibrateTool, &QPushButton::clicked, this, &GUIWindow::handleCalibrateClicked);
    // connect(&cvTimer, &QTimer::timeout, this, &GUIWindow::updateVision);
    
    // Initialize update timer
    windowUpdateTimer.setInterval(100);
    connect(&windowUpdateTimer, &QTimer::timeout, this, &GUIWindow::updateWindow);
    windowUpdateTimer.start();
}

void GUIWindow::updateWindow() {
    setConnectionStatus();
    setTurnStatus();
    gui.setDifficulty(getSliderValue());
}

int GUIWindow::getSliderValue() const {
    return difficultySlider.value();
}

// void GUIWindow::updateVision() {
//     cv::Mat mat = gui.getVision();
//     if (mat.empty()) return;

//     cv::Mat rgb;
//     cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);

//     QImage img((uchar*)rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888);
//     cvFeed = img.copy();
//     cvLabel->setPixmap(QPixmap::fromImage(cvFeed).scaled(800, 400, Qt::KeepAspectRatio, Qt::SmoothTransformation));
// }

void GUIWindow::setTurnStatus() {
    turnStatus.setText(gui.getTurn() == "Robot" ? "Turn: Robot" : "Turn: Player");
}

void GUIWindow::setConnectionStatus() {
    bool connected = gui.getConnection();
    connectionStatus.setTextFormat(Qt::RichText);
    connectionStatus.setText(QString("<span>UR5: </span><span style='color:%1;'>%2</span>").arg(connected ? "green" : "red").arg(connected ? "Connected" : "Disconnected"));
}

void GUIWindow::updateDifficultyLabel(int value) {
    difficultyLabel.setText(QString("Robot Elo raiting: %1").arg(value));
}

void GUIWindow::handleSliderChanged(int value) {
    gui.setDifficulty(value);
    updateDifficultyLabel(value);
    emit difficultyChanged(value);
}

void GUIWindow::handleStartClicked() {
    if (!gui.getGameActive() && !gui.getGameResetting()) {
        gui.changeState("Start");
        emit startClicked();
    }
}

void GUIWindow::handleResetClicked() {
    if (gui.getGameActive()) {
        gui.changeState("Reset");
        emit resetClicked();
    }
}

void GUIWindow::handleCalibrateClicked() {
    gui.changeState("Calibrate");
    emit calibrateClicked();
}

string GUIWindow::selectPawnPromotion() {
    if (QThread::currentThread() == thread()) {
        return selectPawnPromotionImpl().toStdString();
    }
  
    QString result;
    QMetaObject::invokeMethod(this, "selectPawnPromotionImpl", Qt::BlockingQueuedConnection, Q_RETURN_ARG(QString, result));
    return result.toStdString();
  }
  
  QString GUIWindow::selectPawnPromotionImpl() {
    if (promotionBox) return {};
  
    const QString boxStyle = "QGroupBox{ border:1px solid gray; border-radius:10px; padding:10px; margin-top:1ex; background:#f8f8f8; }";
  
    promotionBox = new QGroupBox("Pawn Promotion", this);
    promotionBox->setStyleSheet(boxStyle);
    auto *promoLayout = new QVBoxLayout(promotionBox);
  
    auto *prompt = new QLabel("Select promotion type", promotionBox);
    prompt->setAlignment(Qt::AlignCenter);
    promoLayout->addWidget(prompt);
  
    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    QStringList types = {"Queen","Rook","Bishop","Knight"};
    QEventLoop loop;
    QString selected;
  
    for (auto &t : types) {
        QPushButton *btn = new QPushButton(t, promotionBox);
        connect(btn, &QPushButton::clicked, this, [&](){
            selected = t;
            loop.quit();
        });
        buttonsLayout->addWidget(btn);
    }
  
    promoLayout->addLayout(buttonsLayout);
    mainLayout.addWidget(promotionBox);
  
    loop.exec();
  
    mainLayout.removeWidget(promotionBox);
    promotionBox->deleteLater();
    promotionBox = nullptr;
  
    return selected;
  }