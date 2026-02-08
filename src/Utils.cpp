#include "Utils.hpp"
#include <arpa/inet.h>

namespace Utils {
    uint16_t calculateCheckSum(void* data, size_t lenght) {
        uint8_t* ptr = static_cast<uint8_t*>(data);
        uint32_t sum {0};

        for (size_t i = 0; i < lenght - 1; i+=2) {
            uint16_t word = (static_cast<uint16_t>(ptr[i]) << 8) + ptr[i + 1];
            sum += word;
        }

        if (lenght % 2 != 0) {
            sum += (static_cast<uint16_t>(ptr[lenght - 1]) << 8);
        }

        while (sum >> 16) {
            sum = (sum & 0xFFFF) + (sum >> 16);
        }

        return htons(static_cast<uint16_t>(~sum));

    }
}