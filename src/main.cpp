#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <csignal>
#include "Router.hpp"
#include "TunDevice.hpp"
#include "Logger.hpp"
#include "Exceptions.hpp"

volatile sig_atomic_t keepRunning = 1;

void signalHandler(int) {
    keepRunning = 0;
}

static void installSignalHandlers() {
    struct sigaction action {};
    action.sa_handler = signalHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    if (sigaction(SIGINT, &action, nullptr) != 0 ||
        sigaction(SIGTERM, &action, nullptr) != 0) {
        throw std::runtime_error("Failed to install signal handlers");
    }
}

int main(int argc, char* argv[]) {
    auto& logger = Logging::Logger::getInstance();

    std::string interfaceName = "tun0";

    try {
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];

            if (arg == "-d" || arg == "--debug") {
                logger.setLogLevel(Logging::LogLevel::DEBUG);
            } else if (arg == "-l" || arg == "--logfile") {
                if (i + 1 >= argc) {
                    logger.error("Missing argument for " + arg, "Main");
                    return 1;
                }
                logger.setLogFile(argv[++i]);
                logger.enableFileOutput(true);
            } else if (arg == "-i" || arg == "--interface") {
                if (i + 1 >= argc) {
                    logger.error("Missing argument for " + arg, "Main");
                    return 1;
                }
                interfaceName = argv[++i];
            } else if (arg == "-q" || arg == "--quiet") {
                logger.enableConsoleOutput(false);
            } else if (arg == "-h" || arg == "--help") {
                std::cout << "Usage: LiteRouter [OPTIONS]\n"
                        << "  -d, --debug              Enable debug logging\n"
                        << "  -l, --logfile  <file>    Write logs to file\n"
                        << "  -i, --interface <name>   TUN interface name (default: tun0)\n"
                        << "  -q, --quiet              Disable console output\n"
                        << "  -h, --help               Show this help message\n";
                return 0;
            } else {
                logger.error("Unknown option: " + arg, "Main");
                return 1;
            }
        }

        installSignalHandlers();

        logger.info("LiteRouter starting (ICMP Echo on TUN)");

        TunDevice tun(interfaceName);
        Router router;
        std::vector<unsigned char> buffer(2048);
        
        logger.info("Configure the interface with:", "Main");
        logger.info("  sudo ip link set dev " + interfaceName + " up", "Main");
        logger.info("  sudo ip addr add 10.0.0.2/24 dev " + interfaceName, "Main");
        logger.info("  ping -c 3 10.0.0.1", "Main");
        logger.info("Press Ctrl+C to stop", "Main");
        logger.info("Or use run_test.sh file", "Main");

        while (keepRunning) {
            try {
                ssize_t bytesRead = tun.read(buffer);

                if (bytesRead > 0) {
                    router.handlePacket(buffer, bytesRead, tun);
                }
            } catch (const Exceptions::TunDeviceReadException& e) {
                logger.error(e.what(), "Main");
                
                if (!keepRunning) break;
            }
        }

    } catch (const Exceptions::TunDeviceException& e) {
        logger.critical(e.what(), "Main");
        logger.critical("Make sure you run with sudo/root privileges", "Main");
        return 1;
    } catch (const std::exception& e) {
        logger.critical("Fatal error: " + std::string(e.what()), "Main");
        return 1;
    }

    logger.info("=== Stopped ===", "Main");
    return 0;
}
