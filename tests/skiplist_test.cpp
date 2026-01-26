#include <gtest/gtest.h>
#include "skiplist.h"

#include <algorithm>
#include <random> 
#include <string>
#include <unordered_map>
#include <vector>

/**
 * @file skiplist_test.cpp
 * @brief SkipList 的单元测试（当前覆盖 insert/search）
 *
 * 这些测试的核心目标不是验证“随机性”，而是验证跳表在不同路径下的基础不变量：
 * - 插入后可查到；未插入查不到
 * - 重复插入同 key 表现为更新（以当前实现为准）
 * - 多层 forward 指针路径正确（通过固定概率让层数可控）
 *
 * 为了让测试更稳定、更容易定位问题，这里刻意使用两种极端概率：
 * - prob = 0.0f：random_level() 恒返回 1，退化为“单层有序链表”，便于验证最基础逻辑
 * - prob = 1.0f：random_level() 恒增长到 max_level，强制覆盖“扩层/多层指针维护”路径
 */

/**
 * @brief 空表查找：新建跳表后查找任意 key 都应未命中
 *
 * 这是最基础的冒烟用例，能快速发现 search 在空表边界上的空指针/层级处理问题。
 */
TEST(SkipListTest, InitialStateSearchMiss) { 
    SkipList<int, std::string> kv(16);
    EXPECT_FALSE(kv.search(123).has_value());
}

/**
 * @brief 单条插入与查询：插入后命中；未插入 key 仍未命中
 *
 * 固定 prob=0，使结构退化为单层，避免随机层数干扰，先验收“有序插入 + 精确命中”。
 */
TEST(SkipListTest, InsertAndSearchSingle) {
    SkipList<int, std::string> kv(16, 0.0f);

    EXPECT_TRUE(kv.insert(1, "one"));

    auto v1 = kv.search(1);
    ASSERT_TRUE(v1.has_value());
    EXPECT_EQ(v1.value(), "one");

    EXPECT_FALSE(kv.search(2).has_value());
}

/**
 * @brief 重复 key 的语义：同 key 再次 insert 应更新 value
 *
 * 如果后续你希望“重复 key 插入失败”，只需要同步修改 insert 的实现语义与此测试断言。
 */
TEST(SkipListTest, UpdateExistingKey) {
    SkipList<int, std::string> kv(16, 0.0f);

    EXPECT_TRUE(kv.insert(7, "a"));
    EXPECT_TRUE(kv.insert(7, "b"));

    auto v = kv.search(7);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v.value(), "b");
}

/**
 * @brief 顺序批量插入：插入 [0, n) 后应全部可查，且 value 正确
 *
 * 这个用例在“单层退化形态”下等价于验证有序链表插入的正确性，
 * 很适合先把 insert/search 的最小闭环跑通。
 */
TEST(SkipListTest, InsertManyAndSearchAll_Level1) {
    SkipList<int, std::string> kv(16, 0.0f);

    const int n = 2000;
    for (int i = 0; i < n; ++i) {
        EXPECT_TRUE(kv.insert(i, std::to_string(i)));
    }

    for (int i = 0; i < n; ++i) {
        auto v = kv.search(i);
        ASSERT_TRUE(v.has_value());
        EXPECT_EQ(v.value(), std::to_string(i));
    }

    EXPECT_FALSE(kv.search(-1).has_value());
    EXPECT_FALSE(kv.search(n).has_value());
}

/**
 * @brief 乱序插入：插入顺序与 key 的自然顺序无关，但都必须可查
 *
 * 用 unordered_map 保存“期望值”，避免测试依赖遍历顺序。
 * 同样固定 prob=0，使问题定位更聚焦（逻辑错误更容易复现）。
 */
TEST(SkipListTest, RandomInsertOrderStillSearchable_Level1) {
    SkipList<int, std::string> kv(16, 0.0f);

    const int n = 1000;
    std::vector<int> keys;
    keys.reserve(n);
    for (int i = 0; i < n; ++i) keys.push_back(i);

    std::mt19937 rng(12345);
    std::shuffle(keys.begin(), keys.end(), rng);

    std::unordered_map<int, std::string> expected;
    expected.reserve(n);

    for (int k : keys) {
        std::string val = "v" + std::to_string(k);
        expected.emplace(k, val);
        EXPECT_TRUE(kv.insert(k, val));
    }

    for (const auto& [k, val] : expected) {
        auto got = kv.search(k);
        ASSERT_TRUE(got.has_value());
        EXPECT_EQ(got.value(), val);
    }
}

/**
 * @brief 强制多层路径：prob=1.0f 使所有节点都升到 max_level
 *
 * 该用例主要覆盖：
 * - insert 时 new_level > current_level 的扩层逻辑
 * - 多层 forward 指针维护是否一致
 * - search 从高层向下“下沉”是否正确
 */
TEST(SkipListTest, WorksWhenAllNodesMaxLevel) {
    SkipList<int, int> kv(8, 1.0f);

    for (int i = 1; i <= 200; ++i) {
        EXPECT_TRUE(kv.insert(i, i * 10));
    }

    for (int i = 1; i <= 200; ++i) {
        auto v = kv.search(i);
        ASSERT_TRUE(v.has_value());
        EXPECT_EQ(v.value(), i * 10);
    }

    EXPECT_FALSE(kv.search(0).has_value());
    EXPECT_FALSE(kv.search(201).has_value());
}

/**
 * @brief key 为 std::string 的场景：验证模板与比较逻辑可用
 *
 * 跳表依赖 key 的严格弱序（这里用 std::string 默认字典序），
 * 该用例用于快速发现“比较/模板实例化/默认构造”等相关问题。
 */
TEST(SkipListTest, StringKeyWorks) {
    SkipList<std::string, int> kv(8, 0.0f);

    EXPECT_TRUE(kv.insert("b", 2));
    EXPECT_TRUE(kv.insert("a", 1));
    EXPECT_TRUE(kv.insert("c", 3));

    auto a = kv.search("a");
    auto b = kv.search("b");
    auto c = kv.search("c");
    auto d = kv.search("d");

    ASSERT_TRUE(a.has_value());
    ASSERT_TRUE(b.has_value());
    ASSERT_TRUE(c.has_value());
    EXPECT_FALSE(d.has_value());

    EXPECT_EQ(a.value(), 1);
    EXPECT_EQ(b.value(), 2);
    EXPECT_EQ(c.value(), 3);
}
