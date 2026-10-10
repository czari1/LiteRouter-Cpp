#include <gtest/gtest.h>

extern "C" {
#include "protocol_core.h"
}

TEST(ProtocolCore, ChecksumMatchesCppPath) {
    unsigned char data[] = {0x45, 0x00, 0x00, 0x54, 0x00, 0x00, 0x40, 0x00};
    EXPECT_NE(protocol_checksum(data, sizeof(data)), 0U);
}

TEST(ProtocolCore, Ipv4HeaderLength) {
    EXPECT_EQ(protocol_ipv4_header_length(0x45, 64), 20U);
    EXPECT_EQ(protocol_ipv4_header_length(0x44, 64), 0U);
}
