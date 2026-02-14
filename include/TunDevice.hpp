#pragma once

#include <string>
#include <vector>
#include <unistd.h>
#include "Exceptions.hpp"
#include "Logger.hpp"
class TunDevice {
public:
    explicit TunDevice(const std::string& name);
    
    ~TunDevice();

    TunDevice(const TunDevice&) = delete;
    TunDevice& operator=(const TunDevice&) = delete;

    ssize_t read(std::vector<unsigned char>& buffer);

    ssize_t write(const std::vector<unsigned char>& buffer, size_t size);

    const std::string& getName() const { return m_name; }
private:
    std::string m_name;
    int m_fd;
};