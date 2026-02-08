#pragma once
#include <vector>
#include <iostream>
#include "TunDevice.hpp"
#include "ProtocolDefinition.hpp"

class Router {
public:
    Router() = default;
    void handlePacket(std::vector<unsigned char>& buffer, ssize_t length, TunDevice& device);

private:
    void handleICMP(Net::IPv4Header* ipHeader, std::vector<unsigned char>& buffer, ssize_t length, TunDevice& device);
};