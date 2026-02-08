#pragma once
#include <cstdint>

namespace Net {

    struct __attribute__((packed)) IPv4Header {
        uint8_t version_ihl;
        uint8_t tos;
        uint16_t total_lenght;
        uint16_t ipv4_id;
        uint16_t flags_frag_offset;
        uint8_t ttl;
        uint8_t protocol;
        uint16_t header_checksum;
        uint32_t src_addr;
        uint32_t dest_addr;
        };

    struct __attribute__ ((packed)) ICMPHeader {
        uint8_t type;
        uint8_t code;
        uint16_t checksum;
        uint16_t icmp_id;
        uint16_t sequence;
    };

}

