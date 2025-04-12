#include "PhoneticInput.h"
#include <algorithm>
#include <cctype>
#include <regex>
#include <stdexcept>
#include <thread>
#include <chrono>

PhoneticInput::PhoneticInput() : audioStream(nullptr), model(nullptr), 
                                recognizer(nullptr), isInitialized(false) {
    initializeMaps();
    if (!initializeSpeechRecognition()) {
        throw std::runtime_error("Failed to initialize speech recognition");
    }
}

PhoneticInput::~PhoneticInput() {
    if (audioStream) {
        Pa_CloseStream(audioStream);
        Pa_Terminate();
    }
    if (recognizer) {
        vosk_recognizer_free(recognizer);
    }
    if (model) {
        vosk_model_free(model);
    }
}

bool PhoneticInput::initializeSpeechRecognition() {
    // Initialize PortAudio
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }

    // Open default audio input stream
    err = Pa_OpenDefaultStream(&audioStream,
                             1,          // mono input
                             0,          // no output
                             paFloat32,  // sample format
                             16000,      // sample rate
                             256,        // frames per buffer
                             nullptr,    // no callback
                             nullptr);   // no user data

    if (err != paNoError) {
        std::cerr << "PortAudio stream error: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }

    // Initialize Vosk
    model = vosk_model_new("model"); // Make sure you have the model files
    if (!model) {
        std::cerr << "Could not load Vosk model" << std::endl;
        return false;
    }

    recognizer = vosk_recognizer_new(model, 16000.0);
    if (!recognizer) {
        std::cerr << "Could not create Vosk recognizer" << std::endl;
        return false;
    }

    isInitialized = true;
    return true;
}

std::string PhoneticInput::listenForCommand() {
    if (!isInitialized) {
        throw std::runtime_error("Speech recognition not initialized");
    }

    std::cout << "Listening... Speak your move now." << std::endl;
    
    Pa_StartStream(audioStream);
    
    const int maxDuration = 5; // Maximum listening duration in seconds
    float buffer[256];
    std::string result;
    auto startTime = std::chrono::steady_clock::now();
    
    while (true) {
        // Check if we've exceeded maximum duration
        auto currentTime = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>
            (currentTime - startTime).count() >= maxDuration) {
            break;
        }

        // Read audio data
        PaError err = Pa_ReadStream(audioStream, buffer, 256);
        if (err) continue;

        // Process audio data with Vosk
        if (vosk_recognizer_accept_waveform(recognizer, buffer, 256 * sizeof(float))) {
            const char* json = vosk_recognizer_result(recognizer);
            // Parse JSON result to extract text
            // Note: You'll need to add JSON parsing here
            result = json; // Simplified for example
            if (!result.empty()) {
                break;
            }
        }
    }

    Pa_StopStream(audioStream);
    return result;
}

std::string PhoneticInput::getMove() {
    std::string move;
    bool validMove = false;

    while (!validMove) {
        try {
            // Get source square
            std::cout << "Please speak the source square (e.g., 'ECHO TWO')" << std::endl;
            std::string sourceSquare = listenForCommand();
            
            // Get destination square
            std::cout << "Please speak the destination square (e.g., 'ECHO FOUR')" << std::endl;
            std::string destSquare = listenForCommand();

            // Process and validate the input
            std::string fullMove = sourceSquare + " " + destSquare;
            if (validatePhoneticInput(fullMove)) {
                // Convert phonetic input to chess notation
                move = processAudioInput();
                validMove = true;
            } else {
                std::cout << "Invalid move format. Please try again." << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
            std::cout << "Please try again." << std::endl;
        }
    }

    return move;
}

void PhoneticInput::initializeMaps() {
    // Initialize letter mappings (a-h for chess columns)
    phoneticToChess["alpha"] = 'a';
    phoneticToChess["bravo"] = 'b';
    phoneticToChess["charlie"] = 'c';
    phoneticToChess["delta"] = 'd';
    phoneticToChess["echo"] = 'e';
    phoneticToChess["foxtrot"] = 'f';
    phoneticToChess["golf"] = 'g';
    phoneticToChess["hotel"] = 'h';
    
    // Initialize number mappings (1-8 for chess rows)
    phoneticToNumber["one"] = '1';
    phoneticToNumber["two"] = '2';
    phoneticToNumber["three"] = '3';
    phoneticToNumber["four"] = '4';
    phoneticToNumber["five"] = '5';
    phoneticToNumber["six"] = '6';
    phoneticToNumber["seven"] = '7';
    phoneticToNumber["eight"] = '8';
}

void PhoneticInput::printInstructions() const {
    std::cout << "\nPhonetic Chess Input Instructions:" << std::endl;
    std::cout << "1. Input the source square using phonetic alphabet (e.g., 'echo two')" << std::endl;
    std::cout << "2. Then input the destination square (e.g., 'echo four')" << std::endl;
    std::cout << "Example: For move e2e4, say: 'echo two echo four'" << std::endl;
    std::cout << "\nValid phonetic letters: ALPHA through HOTEL" << std::endl;
    std::cout << "Valid numbers: ONE through EIGHT" << std::endl;
}

std::string PhoneticInput::convertPhoneticToChessNotation(const std::string& phonetic) const {
    std::stringstream ss(phonetic);
    std::string letter, number;
    ss >> letter >> number;
    
    auto letterIt = phoneticToChess.find(letter);
    auto numberIt = phoneticToNumber.find(number);
    
    if (letterIt == phoneticToChess.end() || numberIt == phoneticToNumber.end()) {
        return "";
    }
    
    return std::string(1, letterIt->second) + std::string(1, numberIt->second);
}

bool PhoneticInput::isValidMove(const std::string& move) const {
    if (move.length() != 4) return false;
    
    // Check if the move format is valid (e.g., "e2e4")
    std::regex movePattern("^[a-h][1-8][a-h][1-8]$");
    return std::regex_match(move, movePattern);
}

bool PhoneticInput::validatePhoneticInput(const std::string& input) {
    // Implementation of validatePhoneticInput method
    return true; // Placeholder return, actual implementation needed
}

std::string PhoneticInput::processAudioInput() {
    // Implementation of processAudioInput method
    return ""; // Placeholder return, actual implementation needed
}