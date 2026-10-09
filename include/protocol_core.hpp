#ifndef PROTOCOL_CORE_HPP
#define PROTOCOL_CORE_HPP

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** RFC 1071 internet checksum. Returns 0xFFFF for zero length. */
uint16_t protocol_checksum(void* data, size_t length);

/** Returns header length in bytes, or 0 if IHL or packet length is invalid. */
size_t protocol_ipv4_header_length(uint8_t version_ihl, size_t packet_length);

#ifdef __cplusplus
}
#endif

#endif
