#include "TunDevice.hpp"
#include <iostream>
#include <cstring>      
#include <stdexcept>    
#include <unistd.h>     
#include <fcntl.h>      
#include <sys/ioctl.h>  
#include <net/if.h>     
#include <linux/if_tun.h>

TunDevice::TunDevice(const std::string& name) : m_name(name), m_fd(-1) {
    auto& logger = Logging::Logger::getInstance();

    logger.info("Opening TUN device: " + name, "TunDevice");

    if ((m_fd = ::open("/dev/net/tun", O_RDWR)) < 0) {
        logger.critical("Failed to open /dev/net/tun: " + std::string(strerror(errno)), "TunDevice");
        throw Exceptions::TunDeviceOpenException("/dev/net/tun. Root permission?"); 
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));

    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;

    std::strncpy(ifr.ifr_name, name.c_str(), IFNAMSIZ);

    if (::ioctl(m_fd, TUNSETIFF, (void*) &ifr) < 0) {
        ::close(m_fd);
        m_fd = -1;
        logger.critical("ioctl (TUNSETIFF) failed: " + std::string(strerror(errno)), "TunDevice");
        throw Exceptions::TunDeviceConfigException("ioctl(TUNSETIFF) failed: " + std::string(strerror(errno)));
    }

    logger.info("Interface: " + name + " created successfully.", "TunDevice");
}

TunDevice::~TunDevice() {
    if (m_fd > 0) {
        Logging::Logger::getInstance().info("Closing TUN device: " + m_name, "TunDevice");
        ::close(m_fd);
        m_fd = -1;
    }
}

ssize_t TunDevice::read(std::vector<unsigned char>& buffer) {
    auto& logger = Logging::Logger::getInstance();

    ssize_t bytesRead = ::read(m_fd, buffer.data(), buffer.size());

    if (bytesRead < 0) {

        if (errno == EINTR) {
            logger.debug("Read interrupted by signal", "TunDevice");
            return 0;
        }
        logger.error("Read failed: " + std::string(strerror(errno)), "TunDevice");
        throw Exceptions::TunDeviceReadException(strerror(errno));
    }

    logger.debug("Read " + std::to_string(bytesRead) + "bytes.", "TunDevice");
    
    return bytesRead;
}

ssize_t TunDevice::write(const std::vector<unsigned char>& buffer, size_t size) {
    auto& logger = Logging::Logger::getInstance();

    ssize_t bytesWritten = ::write(m_fd, buffer.data(), size);

    if (bytesWritten < 0) {
        logger.error("Write failed: " + std::string(strerror(errno)), "TunDevice");
        throw Exceptions::TunDeviceWriteException(strerror(errno));
    }

    if (static_cast<size_t>(bytesWritten) != size) {
        logger.warning("Partial write: " + std::to_string(bytesWritten) + "/" + std::to_string(size) + "bytes", "TunDevice");
    } else {
        logger.debug("Wrote " + std::to_string(bytesWritten) + " bytes", "TunDevice");
    }

    return bytesWritten;
}
