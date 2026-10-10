#pragma once
#include <cstdint>
#include <cstddef>
#include <optional>

namespace Utils {
    /** Internet checksum (RFC 1071) in network byte order, for a uint16_t header field. */
    uint16_t calculateCheckSum(void* data, size_t length);

    /** Returns IPv4 header length in bytes, or nullopt if IHL or buffer size is invalid. */
    std::optional<size_t> ipv4HeaderLength(uint8_t version_ihl, size_t packetLength);
}
