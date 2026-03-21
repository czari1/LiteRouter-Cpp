#include "Utils.hpp"
#include <arpa/inet.h>

namespace Utils {
    uint16_t calculateCheckSum(void* data, size_t length    ) {
        uint8_t* ptr = static_cast<uint8_t*>(data);
        uint32_t sum {0};

        for (size_t i = 0; i < length - 1; i+=2) {
            uint16_t word = (static_cast<uint16_t>(ptr[i]) << 8) + ptr[i + 1];
            sum += word;
        }

        if (length % 2 != 0) {
            sum += (static_cast<uint16_t>(ptr[length - 1]) << 8);
        }

        while (sum >> 16) {
            sum = (sum & 0xFFFF) + (sum >> 16);
        }

        return htons(static_cast<uint16_t>(~sum));

    }
}