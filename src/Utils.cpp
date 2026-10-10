#include "Utils.hpp"
#include "protocol_core.hpp"

#include <arpa/inet.h>

namespace Utils {
    uint16_t calculateCheckSum(void* data, size_t length) {
        return htons(protocol_checksum(data, length));
    }

    std::optional<size_t> ipv4HeaderLength(uint8_t version_ihl, size_t packetLength) {
        const size_t headerLen = protocol_ipv4_header_length(version_ihl, packetLength);
        if (headerLen == 0U) {
            return std::nullopt;
        }
        return headerLen;
    }
}
