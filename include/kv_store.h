#pragma once
#define _CRT_SECURE_NO_WARNINGS // 禁用 fopen 等不安全警告

#include "skiplist.h"
#include "wal_record.h"
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>

// 平台兼容性处理：为了调用底层的 Sync API
// Windows 使用 _commit, Linux/Unix 使用 fsync
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

/**
 * @brief 分布式 KV 存储引擎的核心入口类
 *
 * KVStore 负责统一管理内存中的 MemTable (SkipList) 和磁盘上的持久化文件
 * (WAL/SSTable)。 它对外提供 Put/Get/Del 接口，保证数据在写入内存前先落盘（WAL）。
 * 
 * @note 本类非线程安全，多线程环境下需外部加锁。
 */
class KVStore {
public:
  /**
   * @brief 构造函数：初始化存储引擎
   *
   * 初始化流程：
   * 1. 检查数据目录是否存在，若不存在则自动创建。
   * 2. 以 "ab" (Append + Binary) 模式打开 WAL 文件。
   *    - 若文件不存在，fopen 会自动创建。
   *    - 若文件存在，文件指针自动定位到末尾，准备追加写入。
   * 3. 检查 WAL 文件大小：
   *    - 若 > 0：说明是崩溃后重启，触发 Recovery 流程（Task 4 实现）。
   *    - 若 = 0：说明是新实例，无需恢复。
   *
   * @param dir 数据存储目录的路径（如 "./data"）
   * @throw std::runtime_error 如果 WAL 文件无法打开
   */
  explicit KVStore(const std::string &dir) : data_dir_(dir) {
    if (!std::filesystem::exists(data_dir_)) {
      std::filesystem::create_directories(data_dir_);
      std::cout << "[KVStore] Created data directory: "
                << std::filesystem::absolute(data_dir_) << std::endl;
    }

    wal_path_ = data_dir_ / "wal.log";

    // 使用 C 风格 fopen 以便获取底层文件描述符用于 Sync
    wal_file_ = fopen(wal_path_.string().c_str(), "ab");
    if (!wal_file_) {
      throw std::runtime_error("Failed to open WAL file: " +
                               wal_path_.string());
    }

    if (std::filesystem::file_size(wal_path_) > 0) {
      std::cout << "[KVStore] Found existing WAL, ready for recovery (Task 4)."
                << std::endl;
    } else {
      std::cout << "[KVStore] WAL initialized (New)." << std::endl;
    }
  }

  /**
   * @brief 析构函数：资源释放
   * 
   * 关闭 WAL 文件句柄，确保缓冲区数据刷新（虽然 OS 也会做，但显式关闭是好习惯）。
   */
  ~KVStore() {
    if (wal_file_) {
      std::fclose(wal_file_);
      wal_file_ = nullptr;
    }
  }

  /**
   * @brief 写入键值对 (Put)
   *
   * 核心时序 (The Invariant)：
   * 1. Construct: 构造 LogRecord。
   * 2. WAL: 调用 append_wal 写入磁盘并 Sync。
   * 3. MemTable: 更新内存数据结构。
   *
   * @param key 键
   * @param value 值
   * @throw std::runtime_error 如果 WAL 写入失败
   */
  void put(int key, const std::string &value) {
    LogRecord record{LogType::kPut, std::to_string(key), value};
    append_wal(record);
    memtable_.insert(key, value);
  }

  /**
   * @brief 查询键值对 (Get)
   *
   * 当前仅查询内存中的 MemTable。
   *
   * @param key 键
   * @return std::optional<std::string> 若存在返回 value，否则返回 std::nullopt
   */
  std::optional<std::string> get(int key) { return memtable_.search(key); }

  /**
   * @brief 删除键值对 (Delete)
   *
   * 同样遵循 WAL 先行原则：
   * 1. 写入 Delete 类型的日志记录。
   * 2. 在内存中执行删除操作。
   *
   * @param key 键
   * @return true 删除成功（Key 存在且被移除）
   * @return false 删除失败（Key 不存在）
   */
  bool del(int key) {
    LogRecord record{LogType::kDelete, std::to_string(key), ""};
    append_wal(record);
    return memtable_.remove(key);
  }

private:
  /**
   * @brief 核心辅助函数：将日志记录持久化到磁盘
   * 
   * 实现了数据持久化的“三步走”策略，确保 ACID 中的 Durability（持久性）：
   * 
   * 1. **Write (fwrite)**: 
   *    将数据从 Application Buffer 拷贝到 C Library Buffer (用户态)。
   *    速度极快，但这只是内存拷贝，未离开当前进程。
   * 
   * 2. **Flush (fflush)**: 
   *    将数据从 C Library Buffer 刷新到 OS Page Cache (内核态)。
   *    此时如果进程崩溃 (Crash)，数据安全；但如果操作系统崩溃或断电，数据丢失。
   * 
   * 3. **Sync (fsync / _commit)**: 
   *    强制操作系统将 Page Cache 中的数据写入物理磁盘介质。
   *    这是最慢的一步（机械盘 ms 级，SSD us 级），但能保证断电不丢数据。
   * 
   * @param record 待写入的日志记录
   * @throw std::runtime_error 任何一步 I/O 失败都会抛出异常，触发 Fail-Stop
   */
  void append_wal(const LogRecord& record) {
    std::string data = encode_log_record(record);

    // Step 1: Write to user-space buffer
    if(std::fwrite(data.data(), 1, data.size(), wal_file_) != data.size()) {
        throw std::runtime_error("Failed to write to WAL file");
    }

    // Step 2: Flush to kernel buffer
    if(std::fflush(wal_file_) != 0) {
        throw std::runtime_error("WAL flush failed");
    }

    // Step 3: Sync to physical disk
    #ifdef _WIN32
        // Windows: _commit 强制将句柄关联的缓冲区刷入磁盘
        if(_commit(_fileno(wal_file_)) != 0) {
            throw std::runtime_error("WAL commit failed");
        }
    #else
        // Linux/POSIX: fsync 标准接口
        if(fsync(fileno(wal_file_)) != 0) {
            throw std::runtime_error("WAL sync (fsync) failed");
        }
    #endif
  }

private:
  std::filesystem::path data_dir_;       ///< 数据存储根目录
  std::filesystem::path wal_path_;       ///< WAL 预写日志文件的完整路径
  SkipList<int, std::string> memtable_{6}; ///< 内存索引结构（跳表），默认最大层数为 6
  FILE *wal_file_ = nullptr;             ///< WAL 文件句柄 (C-style FILE* for direct sync access)
};
