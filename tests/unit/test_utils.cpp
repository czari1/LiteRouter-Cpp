#include <gtest/gtest.h>
#include "Utils.hpp"
#include "ProtocolDefinition.hpp"

TEST(UtilsChecksum, EmptyBufferReturnsOnesComplement) {
    uint8_t byte = 0;
    EXPECT_EQ(Utils::calculateCheckSum(&byte, 0), 0xFFFF);
}

TEST(UtilsChecksum, SelfConsistentOverBuffer) {
    unsigned char packet[] = {0x08, 0x00, 0x00, 0x00, 0x12, 0x34, 0x00, 0x01};
    const uint16_t computed = Utils::calculateCheckSum(packet, sizeof(packet));
    packet[2] = static_cast<unsigned char>(computed >> 8);
    packet[3] = static_cast<unsigned char>(computed & 0xFF);
    packet[2] = 0;
    packet[3] = 0;
    EXPECT_EQ(Utils::calculateCheckSum(packet, sizeof(packet)), computed);
}

TEST(Ipv4HeaderLength, RejectsIhlBelowFive) {
    EXPECT_FALSE(Utils::ipv4HeaderLength(0x44, 64)); // IHL=4
}

TEST(Ipv4HeaderLength, RejectsHeaderLongerThanPacket) {
    EXPECT_FALSE(Utils::ipv4HeaderLength(0x60, 20)); // IHL=6 needs 24 bytes
}

TEST(Ipv4HeaderLength, AcceptsStandardHeader) {
    const auto len = Utils::ipv4HeaderLength(0x45, 64);
    ASSERT_TRUE(len.has_value());
    EXPECT_EQ(*len, 20U);
}

TEST(ProtocolDefinition, PackedHeaderSize) {
    EXPECT_EQ(sizeof(Net::IPv4Header), 20U);
    EXPECT_EQ(sizeof(Net::ICMPHeader), 8U);
}
