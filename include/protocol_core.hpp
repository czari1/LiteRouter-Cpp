#pragma once

#include <cstddef>
#include <cstdint>

/** RFC 1071 internet checksum in host byte order. Returns 0xFFFF for zero length.
 *  Write it into a packet with htons, or store the high byte first.
 */
uint16_t protocol_checksum(void* data, size_t length);

/** Returns header length in bytes, or 0 if IHL or packet length is invalid. */
size_t protocol_ipv4_header_length(uint8_t version_ihl, size_t packet_length);
