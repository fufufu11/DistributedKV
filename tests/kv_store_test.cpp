#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include "kv_store.h"

namespace fs = std::filesystem;

class KVStoreTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 每个测试开始前，确保数据目录是空的
        test_dir_ = "test_data_kvstore";
        if (fs::exists(test_dir_)) {
            fs::remove_all(test_dir_);
        }
    }

    void TearDown() override {
        // 测试结束后清理（可选，为了观察结果也可以不清理，这里选择清理保持环境干净）
        if (fs::exists(test_dir_)) {
            fs::remove_all(test_dir_);
        }
    }

    std::string test_dir_;
};

/**
 * @brief 测试数据目录的自动创建
 */
TEST_F(KVStoreTest, CreatesDataDirectory) {
    EXPECT_FALSE(fs::exists(test_dir_));
    
    {
        KVStore store(test_dir_);
        EXPECT_TRUE(fs::exists(test_dir_));
        EXPECT_TRUE(fs::is_directory(test_dir_));
    }
}

/**
 * @brief 测试基本的 Put/Get 流程
 */
TEST_F(KVStoreTest, BasicPutGet) {
    KVStore store(test_dir_);
    
    store.put(1, "one");
    store.put(2, "two");

    auto v1 = store.get(1);
    ASSERT_TRUE(v1.has_value());
    EXPECT_EQ(v1.value(), "one");

    auto v2 = store.get(2);
    ASSERT_TRUE(v2.has_value());
    EXPECT_EQ(v2.value(), "two");

    EXPECT_FALSE(store.get(3).has_value());
}

/**
 * @brief 测试更新已存在的 Key
 */
TEST_F(KVStoreTest, UpdateKey) {
    KVStore store(test_dir_);
    store.put(1, "v1");
    EXPECT_EQ(store.get(1).value(), "v1");

    store.put(1, "v1_updated");
    EXPECT_EQ(store.get(1).value(), "v1_updated");
}

/**
 * @brief 测试删除逻辑
 */
TEST_F(KVStoreTest, DeleteKey) {
    KVStore store(test_dir_);
    store.put(10, "ten");
    
    EXPECT_TRUE(store.del(10));
    EXPECT_FALSE(store.get(10).has_value());
    
    // 删除不存在的 key 应返回 false
    EXPECT_FALSE(store.del(10));
}

/**
 * @brief 测试 WAL 文件检测（模拟）
 * 
 * 真正的 WAL 写入和重放将在后续任务实现，这里仅测试“KVStore 能感知到 WAL 文件的存在”
 * 并打印对应的日志（虽然 GTest 捕获 stdout 比较麻烦，但我们可以侧面验证没有崩溃）。
 */
TEST_F(KVStoreTest, DetectsExistingWAL) {
    // 1. 手动创建目录和 WAL 文件
    fs::create_directories(test_dir_);
    std::ofstream wal(fs::path(test_dir_) / "wal.log");
    wal << "dummy content";
    wal.close();

    // 2. 启动 Store，它应该检测到 WAL 并打印 "Found WAL file..."
    // (此处我们主要验证程序能正常启动且不报错)
    {
        KVStore store(test_dir_);
        // 验证目录和文件依然存在（没有被误删）
        EXPECT_TRUE(fs::exists(fs::path(test_dir_) / "wal.log"));
    }
}
