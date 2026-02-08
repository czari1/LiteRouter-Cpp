#include "Router.hpp"
#include "Utils.hpp"
#include <iostream>
#include <arpa/inet.h>
#include <format>

std::string ipToString(uint32_t ipAddress) {
    char buffer[INET_ADDRSTRLEN];
    struct in_addr addr;
    addr.s_addr = ipAddress;
    
    if(inet_ntop(AF_INET, &addr, buffer, INET_ADDRSTRLEN)) {
        return std::string(buffer);
    }
    return "Invalid IP";
}

void Router::handlePacket(std::vector<unsigned char>& buffer, ssize_t length, TunDevice& device) {
    
    if (static_cast<size_t>(length) < sizeof(Net::IPv4Header)) return;

    Net::IPv4Header* ipHeader = reinterpret_cast<Net::IPv4Header*>(buffer.data());

    if (ipHeader->protocol == 1) {
        
        std::cout << "[Router] ICMP Packet: " << ipToString(ipHeader->src_addr) 
                << " -> " << ipToString(ipHeader->dest_addr) << std::endl;
        
        handleICMP(ipHeader, buffer, length, device);
        
    } else {
        std::cout << std::format("This protocol {} is not supported yet \n", ipHeader->protocol);
    }
}

void Router::handleICMP(Net::IPv4Header* ipHeader, std::vector<unsigned char>& buffer, ssize_t length, TunDevice& device) {
    size_t ipHeaderLen = (ipHeader->version_ihl & 0x0F) * 4;
    size_t requiredLen = ipHeaderLen + sizeof(Net::ICMPHeader);
    
    if(static_cast<size_t>(length) < requiredLen) {
        std::cout << "   [ERROR] Packet too short! Received: " << length 
                << ", Required: " << requiredLen << std::endl;
        
                return;
    }

    Net::ICMPHeader* icmpHeader = reinterpret_cast<Net::ICMPHeader*>(buffer.data() + ipHeaderLen);

    if (icmpHeader->type == 8) {
        std::cout << "   -> Ping request detected! Replying.." << std::endl;
        
        uint32_t tempIp = ipHeader->src_addr;
        ipHeader->src_addr = ipHeader->dest_addr;
        ipHeader->dest_addr = tempIp;

        icmpHeader->type = 0;
        icmpHeader->checksum = 0;
        size_t icmpDataLength = length - ipHeaderLen;
        
        icmpHeader->checksum = Utils::calculateCheckSum(icmpHeader, icmpDataLength);
        
        ipHeader->header_checksum = 0;
        ipHeader->header_checksum = Utils::calculateCheckSum(ipHeader, ipHeaderLen);

        device.write(buffer, length);

    } else {
        std::cout << "   [INFO] Ignored ICMP type: " << (int)icmpHeader->type << std::endl;
    }
}