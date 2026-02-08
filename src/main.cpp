#include <iostream>
#include <vector>
#include <csignal>
#include <unistd.h>
#include <format>
#include "Router.hpp"
#include "TunDevice.hpp"

volatile sig_atomic_t keepRunning = 1;

void signalHandler(int) {
    keepRunning = 0;
}

int main() {
    std::signal(SIGINT, signalHandler);

    try {
        std::cout << "Router start \n";
        std::cout << "0";
        TunDevice tun("tun0");
        std::cout << "1";
        Router router;
        std::cout << "2";
        std::vector<unsigned char> buffer(2048);
        std::cout << "3";
        std::cout << std::format("Device {} is ready \n", tun.getName());
        std::cout << "Launch in second terminal this command: sudo ip link set dev tun0 up && sudo ip addr add 10.0.0.1/24 dev tun0\n";
        std::cout << "Press Ctrl+C to kill the process\n";

        while (keepRunning) {
            ssize_t bytesRead = tun.read(buffer);

            if (bytesRead < 0) break;

            if (bytesRead > 0) {
                router.handlePacket(buffer, bytesRead, tun);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[Error]" << e.what() << "\n";
        return 1;
    }
    
    std::cout << "Closing router \n";
    return 0;
}
