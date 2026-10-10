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

static void verifyIpHeaderChecksum(Net::IPv4Header* ipHeader, size_t ipHeaderLen) {
    const uint16_t stored = ipHeader->header_checksum;
    ipHeader->header_checksum = 0;
    const uint16_t computed = Utils::calculateCheckSum(ipHeader, ipHeaderLen);
    ipHeader->header_checksum = stored;

    if (computed != stored) {
        throw Exceptions::ChecksumMismatchException(stored, computed);
    }
}

static void verifyIcmpChecksum(Net::ICMPHeader* icmpHeader, size_t icmpLength) {
    const uint16_t stored = icmpHeader->checksum;
    icmpHeader->checksum = 0;
    const uint16_t computed = Utils::calculateCheckSum(icmpHeader, icmpLength);
    icmpHeader->checksum = stored;

    if (computed != stored) {
        throw Exceptions::ChecksumMismatchException(stored, computed);
    }
}

void Router::handlePacket(std::vector<unsigned char>& buffer, ssize_t length, TunDevice& device) {
    auto& logger = Logging::Logger::getInstance();

    try {
        if (length <= 0 || static_cast<size_t>(length) < sizeof(Net::IPv4Header)) {
            const size_t received = length > 0 ? static_cast<size_t>(length) : 0U;
            throw Exceptions::PacketTooShortException(received, sizeof(Net::IPv4Header));
        }

        Net::IPv4Header* ipHeader = reinterpret_cast<Net::IPv4Header*>(buffer.data());

        const uint8_t version = (ipHeader->version_ihl >> 4) & 0x0F;
        if (version != 4) {
            logger.warning("Non IPv4 packet received (version: " + std::to_string(version) + ")", "Router");
            return;
        }

        const auto ipHeaderLenOpt = Utils::ipv4HeaderLength(ipHeader->version_ihl,
                                                            static_cast<size_t>(length));
        if (!ipHeaderLenOpt) {
            throw Exceptions::InvalidPacketException("Invalid IPv4 IHL or header longer than packet");
        }
        const size_t ipHeaderLen = *ipHeaderLenOpt;

        const uint16_t totalLength = ntohs(ipHeader->total_length);
        if (totalLength < ipHeaderLen || static_cast<size_t>(totalLength) > static_cast<size_t>(length)) {
            throw Exceptions::InvalidPacketException("IPv4 total length does not match the received packet");
        }
        length = static_cast<ssize_t>(totalLength);

        verifyIpHeaderChecksum(ipHeader, ipHeaderLen);

        const uint16_t fragmentation = ntohs(ipHeader->flags_frag_offset);
        const bool moreFragments = (fragmentation & 0x2000U) != 0;
        const bool hasOffset = (fragmentation & 0x1FFFU) != 0;
        if (moreFragments || hasOffset) {
            logger.debug("Ignoring fragmented IPv4 packet", "Router");
            return;
        }

        if (ipHeader->protocol == 1) {
            logger.info("ICMP packet: " + ipToString(ipHeader->src_addr) +
                        " -> " + ipToString(ipHeader->dest_addr), "Router");
            handleICMP(ipHeader, buffer, length, device, ipHeaderLen);
        } else {
            throw Exceptions::UnsupportedProtocolException(ipHeader->protocol);
        }
    } catch (const Exceptions::PacketTooShortException& e) {
        logger.warning(e.what(), "Router");
    } catch (const Exceptions::UnsupportedProtocolException& e) {
        logger.debug(e.what(), "Router");
    } catch (const Exceptions::ChecksumMismatchException& e) {
        logger.warning(e.what(), "Router");
    } catch (const Exceptions::PacketException& e) {
        logger.error(e.what(), "Router");
    }
}

void Router::handleICMP(Net::IPv4Header* ipHeader, std::vector<unsigned char>& buffer,
                        ssize_t length, TunDevice& device, size_t ipHeaderLen) {
    auto& logger = Logging::Logger::getInstance();
    
    try {
        size_t requiredLen = ipHeaderLen + sizeof(Net::ICMPHeader);
        
        if(static_cast<size_t>(length) < requiredLen) {
            throw Exceptions::PacketTooShortException(length, requiredLen);
        }

        Net::ICMPHeader* icmpHeader = reinterpret_cast<Net::ICMPHeader*>(buffer.data() + ipHeaderLen);
        const size_t icmpLength = static_cast<size_t>(length) - ipHeaderLen;

        if (icmpHeader->type == 8) {
            verifyIcmpChecksum(icmpHeader, icmpLength);

            logger.info("Echo Request (Ping) - ID: " + std::to_string(ntohs(icmpHeader->icmp_id)) +
                        ", Seq: " + std::to_string(ntohs(icmpHeader->sequence)), "ICMP");
            
            uint32_t tempIp = ipHeader->src_addr;
            ipHeader->src_addr = ipHeader->dest_addr;
            ipHeader->dest_addr = tempIp;

            icmpHeader->type = 0;
            icmpHeader->checksum = 0;
            
            icmpHeader->checksum = Utils::calculateCheckSum(icmpHeader, icmpLength);
            
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
