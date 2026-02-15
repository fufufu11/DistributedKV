#pragma once

#include <cstdint>
#include <string>
#include <string_view>

struct BlockHandle {
    uint64_t offset = 0;
    uint64_t size = 0;
};

/**
 * @brief Footer 位于 SSTable 文件的末尾，长度固定。
 * 
 * 它的作用是作为文件的“入口”。打开文件时，先读取最后 48 字节（Footer），
 * 从中获取 Index Block 的位置，进而加载索引，最后才能查找数据。
 */
struct Footer {
    // 指向 Metaindex Block (用于存储 Bloom Filter 等元数据，本周暂不实现，留空)
    BlockHandle metaindex_handle;

    // 指向 Index Block (用于存储 Data Block 的索引)
    BlockHandle index_handle;

    // 魔数：用于校验文件是否为合法的 SSTable 文件
    // 0xdb4775248b80fb57 (Little Endian: 57 fb 80 8b 24 75 47 db)
    // 来源于 "DB" + 随机数，防止误读取非 SSTable 文件
    static constexpr uint64_t kTableMagicNumber = 0xdb4775248b80fb57ull;

    // 编码后的固定长度 (2个 Handle + 1个 Magic)
    // Handle 变长编码最大 20 字节 * 2 + 8 = 48 字节
    enum { kEncodedLength = 48 };
};
