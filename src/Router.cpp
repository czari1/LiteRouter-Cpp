#include "Router.hpp"
#include "Utils.hpp"
#include <arpa/inet.h>

std::string ipToString(uint32_t ipAddress) {
    char buffer[INET_ADDRSTRLEN];
    struct in_addr addr;
    addr.s_addr = ipAddress;
    
    if(inet_ntop(AF_INET, &addr, buffer, INET_ADDRSTRLEN)) {
        return std::string(buffer);
    }
    throw Exceptions::InvalidPacketException("Cannot convert IP address to string");
}

void Router::handlePacket(std::vector<unsigned char>& buffer, ssize_t length, TunDevice& device) {
    auto& logger = Logging::Logger::getInstance();

    try {
        if (static_cast<size_t>(length) < sizeof(Net::IPv4Header)) {
            throw Exceptions::PacketTooShortException(length, sizeof(Net::IPv4Header));
        }

        Net::IPv4Header* ipHeader = reinterpret_cast<Net::IPv4Header*>(buffer.data());
        
        uint8_t version = (ipHeader->version_ihl >> 4) & 0x0F;

        if (version != 4) {
            logger.warning("Non IPv4 packet received (version: " + std::to_string(version) + ")", "Router");
            return;
        }

        if (ipHeader->protocol == 1) {
            logger.info("ICMP packet: " + ipToString(ipHeader->src_addr) +
        "->" + ipToString(ipHeader->dest_addr), "Router");
        handleICMP(ipHeader, buffer, length, device);

        } else {
            throw Exceptions::UnsupportedProtocolException(ipHeader->protocol);
        }
    } catch (const Exceptions::PacketTooShortException& e) {
        logger.warning(e.what(), "Router");
    } catch (const Exceptions::UnsupportedProtocolException& e) {
        logger.debug(e.what(), "Router");
    } catch (const Exceptions::PacketException& e) {
        logger.error(e.what(), "Router");
    }
}

void Router::handleICMP(Net::IPv4Header* ipHeader, std::vector<unsigned char>& buffer, ssize_t length, TunDevice& device) {
    auto& logger = Logging::Logger::getInstance();
    
    try {
        size_t ipHeaderLen = (ipHeader->version_ihl & 0x0F) * 4;
        size_t requiredLen = ipHeaderLen + sizeof(Net::ICMPHeader);
        
        if(static_cast<size_t>(length) < requiredLen) {
            throw Exceptions::PacketTooShortException(length, requiredLen);
        }

        Net::ICMPHeader* icmpHeader = reinterpret_cast<Net::ICMPHeader*>(buffer.data() + ipHeaderLen);

        if (icmpHeader->type == 8) {
            logger.info("Echo Request (Ping) - ID: " + std::to_string(ntohs(icmpHeader->icmp_id)) +
                        ", Seq: " + std::to_string(ntohs(icmpHeader->sequence)), "ICMP");
            
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

            logger.info("Echo reply sent to: " + ipToString(ipHeader->dest_addr), "ICMP");

        } else if (icmpHeader->type == 0) {
            logger.debug("Echo Reply received (ignoring)", "ICMP");
        } else {
            logger.debug("ICMP type " + std::to_string(icmpHeader->type) + " not handled", "ICMP");
        }

    } catch (const Exceptions::PacketException& e) {
        logger.warning(e.what(), "ICMP");
    }
}