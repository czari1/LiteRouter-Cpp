#include "Utils.hpp"
#include <arpa/inet.h>

namespace Utils {
    uint16_t calculateCheckSum(void* data, size_t length) {
        if (length == 0) {
            return 0xFFFF;
        }

        auto* ptr = static_cast<uint8_t*>(data);
        uint32_t sum {0};

        for (size_t i = 0; i + 1 < length; i += 2) {
            uint16_t word = (static_cast<uint16_t>(ptr[i]) << 8) + ptr[i + 1];
            sum += word;
        }

        if (length % 2 != 0) {
            sum += static_cast<uint16_t>(ptr[length - 1]) << 8;
        }

        while (sum >> 16) {
            sum = (sum & 0xFFFF) + (sum >> 16);
        }

        return htons(static_cast<uint16_t>(~sum));
    }

    std::optional<size_t> ipv4HeaderLength(uint8_t version_ihl, size_t packetLength) {
        const uint8_t ihl = version_ihl & 0x0F;
        if (ihl < 5) {
            return std::nullopt;
        }
        const size_t headerLen = static_cast<size_t>(ihl) * 4U;
        if (headerLen > packetLength) {
            return std::nullopt;
        }
        return headerLen;
    }
}
