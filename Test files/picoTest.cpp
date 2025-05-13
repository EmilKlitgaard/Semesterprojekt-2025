#include <iostream>
#include <string>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <cstring>

using namespace std;

#define SERIAL_PORT "/dev/ttyACM0"

int main() {
    // Open serial port
    int serial_fd = open(SERIAL_PORT, O_RDWR | O_NOCTTY);
    if (serial_fd == -1) {
        cerr << "Failed to open serial port " << SERIAL_PORT << endl;
        return -1;
    }

    // Configure serial port
    struct termios options;
    tcgetattr(serial_fd, &options);

    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);

    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;        // 8 data bits
    options.c_cflag &= ~PARENB;    // No parity
    options.c_cflag &= ~CSTOPB;    // 1 stop bit
    options.c_cflag &= ~CRTSCTS;   // No flow control
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // Raw input
    options.c_iflag &= ~(IXON | IXOFF | IXANY);         // No software flow control
    options.c_oflag &= ~OPOST;                          // Raw output

    tcsetattr(serial_fd, TCSANOW, &options);

    // Write "Open" to Pico to initiate communication
    string msg = "Close\n";
    write(serial_fd, msg.c_str(), msg.length());
    tcdrain(serial_fd); // Wait until output is sent

    char buffer[256];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int n = read(serial_fd, buffer, sizeof(buffer) - 1);
        
        if (n > 0) {
            buffer[n] = '\0'; // Null-terminate
            string input(buffer);

            if (input.find("Ack") != string::npos) {
                cout << "Found: Ack" << endl;
            }
            if (input.find("Ok") != string::npos) {
                cout << "Found: Ok" << endl;
                close(serial_fd);
                return 0;
            }
            if (input.find("Current:") != string::npos) {
                cout << input;
            }
        } else if (n == 0) {
            cout << "[No data yet]" << endl;
        } else {
            perror("read");
        }

        this_thread::sleep_for(chrono::milliseconds(100));
    }

    close(serial_fd);
    return 0;
}

