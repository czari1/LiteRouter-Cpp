#include <gtest/gtest.h>

#include "Utils.hpp"
#include "protocol_core.hpp"

#include <arpa/inet.h>

TEST(ProtocolCore, ChecksumMatchesRfc1071) {
    // RFC 1071: 0001 F203 F4F5 F6F7 -> checksum 0x220D.
    unsigned char data[] = {0x00, 0x01, 0xF2, 0x03, 0xF4, 0xF5, 0xF6, 0xF7};
    EXPECT_EQ(protocol_checksum(data, sizeof(data)), 0x220D);
    EXPECT_EQ(Utils::calculateCheckSum(data, sizeof(data)), htons(0x220D));
}

TEST(ProtocolCore, ChecksumOfCompletedBufferIsZero) {
    unsigned char packet[] = {0x08, 0x00, 0x00, 0x00, 0x12, 0x34, 0x00, 0x01};
    const uint16_t sum = protocol_checksum(packet, sizeof(packet));
    packet[2] = static_cast<unsigned char>(sum >> 8);
    packet[3] = static_cast<unsigned char>(sum & 0xFF);
    EXPECT_EQ(protocol_checksum(packet, sizeof(packet)), 0);
}

TEST(ProtocolCore, OddLengthChecksum) {
    unsigned char data[] = {0xAB};
    EXPECT_EQ(protocol_checksum(data, sizeof(data)), 0x54FF);
}

TEST(ProtocolCore, Ipv4HeaderLength) {
    EXPECT_EQ(protocol_ipv4_header_length(0x45, 64), 20U);
    EXPECT_EQ(protocol_ipv4_header_length(0x44, 64), 0U);
}
