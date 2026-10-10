#include "protocol_core.hpp"

uint16_t protocol_checksum(void* data, size_t length) {
    if (length == 0U) {
        return 0xFFFFU;
    }

    auto* ptr = static_cast<uint8_t*>(data);
    uint32_t sum = 0U;

    for (size_t i = 0U; i + 1U < length; i += 2U) {
        const uint16_t word = static_cast<uint16_t>(
            (static_cast<uint16_t>(ptr[i]) << 8) | ptr[i + 1]);
        sum += word;
    }

    if ((length % 2U) != 0U) {
        sum += static_cast<uint16_t>(static_cast<uint16_t>(ptr[length - 1U]) << 8);
    }

    while (sum >> 16) {
        sum = (sum & 0xFFFFU) + (sum >> 16);
    }

    return static_cast<uint16_t>(~sum);
}

size_t protocol_ipv4_header_length(uint8_t version_ihl, size_t packet_length) {
    const uint8_t ihl = static_cast<uint8_t>(version_ihl & 0x0FU);
    if (ihl < 5U) {
        return 0U;
    }
    const size_t header_len = static_cast<size_t>(ihl) * 4U;
    if (header_len > packet_length) {
        return 0U;
    }
    return header_len;
}
