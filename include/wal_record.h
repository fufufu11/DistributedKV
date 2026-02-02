#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <cstring> // for memcpy


inline uint32_t crc32(const char* data, size_t lenth) {
    uint32_t crc = 0xFFFFFFFF;
    for(size_t i = 0; i < lenth; i++) {
        uint8_t byte = static_cast<uint8_t> (data[i]);
        crc = crc ^ byte;
        for(int j = 0; j < 8; j++) {
            uint32_t mask = -(crc & 1);
            crc = (crc >> 1) ^ (0xEDB88320 & mask);
        }
    }
    return ~crc;
}
