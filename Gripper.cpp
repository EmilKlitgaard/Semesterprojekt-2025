#include "Gripper.h"

using namespace std;

Gripper::Gripper(const string& portName) {
    serial_fd = open(portName.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (serial_fd < 0) {
        throw runtime_error("Failed to open serial port: " + portName);
    }
    if (!configurePort()) {
        close(serial_fd);
        throw runtime_error("Failed to configure serial port");
    }
}

Gripper::~Gripper() {
    stopGripper();
    if (serial_fd >= 0) close(serial_fd);
}

bool Gripper::configurePort() {
    struct termios options;
    if (tcgetattr(serial_fd, &options) != 0) return false;

    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);

    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;                             // 8 data bits
    options.c_cflag &= ~PARENB;                         // No parity
    options.c_cflag &= ~CSTOPB;                         // 1 stop bit
    options.c_cflag &= ~CRTSCTS;                        // No hardware flow control
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // Raw input
    options.c_iflag &= ~(IXON | IXOFF | IXANY);         // No software flow control
    options.c_oflag &= ~OPOST;                          // Raw output

    return (tcsetattr(serial_fd, TCSANOW, &options) == 0);
}

bool Gripper::sendRaw(const string& cmd) {
    string msg = cmd + "\n";
    write(serial_fd, msg.c_str(), msg.length());
    tcdrain(serial_fd); // Wait until output is sent
    return (tcdrain(serial_fd) == 0);
}

bool Gripper::waitFor(const string& token, double timeoutSeconds) {
    string buffer;
    char chunk[128];
    auto start = chrono::steady_clock::now();

    while (true) {
        int n = read(serial_fd, chunk, sizeof(chunk));
        if (n > 0) {
            buffer.append(chunk, n);
            if (buffer.find(token) != string::npos) {
                return true;
            }
        } else if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            return false;
        }

        auto now = chrono::steady_clock::now();
        if (chrono::duration<double>(now - start).count() > timeoutSeconds) {
            return false;
        }
        this_thread::sleep_for(chrono::milliseconds(10));
    }
}

void Gripper::openGripper() {
    if (!sendRaw("Open")) throw runtime_error("Failed to send Open command");
    if (!waitFor("Ack", 2.0)) throw runtime_error("Timeout waiting for Ack on Open");
    if (!waitFor("Ok", 30.0)) throw runtime_error("Timeout waiting for Ok on Open");
    cout << "Successfully opened gripper" << endl;
}

void Gripper::closeGripper() {
    if (!sendRaw("Close")) throw runtime_error("Failed to send Close command");
    if (!waitFor("Ack", 2.0)) throw runtime_error("Timeout waiting for Ack on Close");
    if (!waitFor("Ok", 30.0)) throw runtime_error("Timeout waiting for Ok on Close");
    cout << "Successfully closed gripper" << endl;
}

void Gripper::stopGripper() {
    if (!sendRaw("Stop")) throw runtime_error("Failed to send Open command");
    cout << "Successfully stopped gripper" << endl;
}