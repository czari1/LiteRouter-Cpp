#include <string>
#include <vector>
#include <csignal>
#include "Router.hpp"
#include "TunDevice.hpp"
#include "Logger.hpp"
#include "Exceptions.hpp"

volatile sig_atomic_t keepRunning = 1;

void signalHandler(int signal) {
    Logging::Logger::getInstance().info("Received signal " 
        + std::to_string(signal) + " shutting down...", "Main");
    keepRunning = 0;
}

int main(int argc, char* argv[]) {
    auto& logger = Logging::Logger::getInstance();

    std::string interfaceName = "tun0";

    for (size_t i = 1; 1 < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-d" || arg == "--debug") {
            logger.setLogLevel(Logging::LogLevel::DEBUG);
        } else if ((arg == "-l" || arg == "--logfile") && i + 1 < argc) {
            logger.setLogFile(argv[++i]);
            logger.enableFileOutput(true);
        } else if ((arg == "-i" || arg == "--interface") && i + 1 < argc) {
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

    try {
        logger.info("LiteRouter starting");

        TunDevice tun(interfaceName);
        Router router;
        std::vector<unsigned char> buffer(2048);
        
        logger.info("Configure the interface with:", "Main");
        logger.info("  sudo ip link set dev " + interfaceName + " up", "Main");
        logger.info("  sudo ip addr add 10.0.0.1/24 dev " + interfaceName, "Main");
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

    logger.info("=== Router Stopped ===", "Main");
    return 0;
}