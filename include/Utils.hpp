#pragma once
#include <cstdint>
#include <cstddef>

namespace Utils {
    uint16_t calculateCheckSum(void* data, size_t lenght);
}