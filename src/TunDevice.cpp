#include "TunDevice.hpp"
#include <iostream>
#include <cstring>      
#include <stdexcept>    
#include <unistd.h>     
#include <fcntl.h>      
#include <sys/ioctl.h>  
#include <net/if.h>     
#include <linux/if_tun.h>
#include <format>

TunDevice::TunDevice(const std::string& name) : m_name(name), m_fd(-1) {

    if ((m_fd = ::open("/dev/net/tun", O_RDWR)) < 0) {
        throw std::runtime_error("Opening error /dev/net/tun. Root permission?");
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));

    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;

    std::strncpy(ifr.ifr_name, name.c_str(), IFNAMSIZ);

    if (::ioctl(m_fd, TUNSETIFF, (void*) &ifr)) {
        ::close(m_fd);
        throw std::runtime_error("ioctl error(TUNSETIFF). Cannot open an interface");
    }

    std::cout << std::format("Interface was created {} \n", name); 
}

TunDevice::~TunDevice() {
    if (m_fd > 0) {
        ::close(m_fd);
    }
}

ssize_t TunDevice::read(std::vector<unsigned char>& buffer) {
    return ::read(m_fd, buffer.data(), buffer.size());
}

ssize_t TunDevice::write(const std::vector<unsigned char>& buffer, size_t size) {
    return ::write(m_fd, buffer.data(), size);
}
