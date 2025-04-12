#pragma once
#include <string>
#include <map>
#include <iostream>
#include <portaudio.h>
#include <vosk_api.h>
#include <memory>

class PhoneticInput {
public:
    PhoneticInput();
    ~PhoneticInput();
    std::string getMove();
    bool initializeSpeechRecognition();

private:
    std::map<std::string, char> letterMap;
    std::map<std::string, char> numberMap;
    
    // Speech recognition members
    PaStream* audioStream;
    VoskModel* model;
    VoskRecognizer* recognizer;
    bool isInitialized;
    
    void initializeMaps();
    std::string listenForCommand();
    std::string processAudioInput();
    bool validatePhoneticInput(const std::string& input);
};