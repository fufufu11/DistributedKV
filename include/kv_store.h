#pragma once

#include <filesystem>
#include <string>
#include <iostream>
#include <optional>
#include "skiplist.h"

/**
 * @brief 分布式 KV 存储引擎的核心入口类
 * 
 * KVStore 负责统一管理内存中的 MemTable (SkipList) 和磁盘上的持久化文件 (WAL/SSTable)。
 * 它对外提供 Put/Get/Del 接口，对内处理数据目录的初始化与崩溃恢复。
 */
class KVStore {
public:
    /**
     * @brief 构造函数：初始化存储引擎
     * 
     * 初始化时会执行以下操作：
     * 1. 检查数据目录是否存在，若不存在则自动创建。
     * 2. 检查 WAL 日志文件是否存在，若存在则触发恢复流程（Recovery）。
     * 
     * @param dir 数据存储目录的路径（如 "./data"）
     */
    explicit KVStore(const std::string& dir) : data_dir_(dir) {
        if (!std::filesystem::exists(data_dir_)) {
            std::filesystem::create_directories(data_dir_);
            std::cout << "[KVStore] Created data directory: " << std::filesystem::absolute(data_dir_) << std::endl;
        }

        wal_path_ = data_dir_ / "wal.log";

        if (std::filesystem::exists(wal_path_)) {
            std::cout << "[KVStore] Found WAL file: " << wal_path_ << ", starting recovery..." << std::endl;
            // TODO: 任务四实现具体的 Replay 逻辑
        } else {
            std::cout << "[KVStore] No WAL found, starting fresh." << std::endl;
        }
    }

    /**
     * @brief 写入键值对
     * 
     * 遵循 WAL 协议：先将操作写入日志，落盘成功后再更新内存。
     * 
     * @param key 键
     * @param value 值
     */
    void put(int key, const std::string& value) {
        // TODO: 任务三实现 WAL 写入（先写日志，再写内存）
        memtable_.insert(key, value);
    }

    /**
     * @brief 查询键值对
     * 
     * 当前仅查询内存中的 MemTable。
     * 
     * @param key 键
     * @return std::optional<std::string> 若存在返回 value，否则返回 std::nullopt
     */
    std::optional<std::string> get(int key) {
        return memtable_.search(key);
    }

    /**
     * @brief 删除键值对
     * 
     * 遵循 WAL 协议：先将删除操作写入日志，落盘成功后再更新内存。
     * 
     * @param key 键
     * @return true 删除成功（Key 存在且被移除）
     * @return false 删除失败（Key 不存在）
     */
    bool del(int key) {
        // TODO: 任务三实现 WAL 删除
        return memtable_.remove(key);
    }

private:
    std::filesystem::path data_dir_;       ///< 数据存储根目录
    std::filesystem::path wal_path_;       ///< WAL 预写日志文件的完整路径
    SkipList<int, std::string> memtable_{6}; ///< 内存索引结构（跳表），默认最大层数为 6
};
