#pragma once
#include <vector>
#include <memory>
#include <random>
#include <optional>
/**
 * @brief 跳表节点结构体
 * @tparam K 键类型
 * @tparam V 值类型
 */
template <typename K, typename V>
struct Node {
    K key;      // 存储的键
    V value;    // 存储的值

    /**
     * @brief 前进指针数组 (Forward List)
     * forward[i] 存储的是该节点在第 i 层的下一个节点的指针
     */
    std::vector<Node<K, V>*> forward;

    /**
     * @brief Node 构造函数
     * @param k 键
     * @param v 值
     * @param level 节点的高度（层数）
     */
    Node(K k, V v, int level)
        : key(k), value(v), forward(level, nullptr) {}
};

/**
 * @brief 跳表类模板
 */
template <typename K, typename V>
class SkipList {
private:
    int max_level;      // 跳表允许的最大层数
    int current_level;  // 当前跳表实际存在的最高层数
    Node<K, V>* head;   // 跳表的头节点（哨兵节点）
    float p;            // 节点层数晋升的概率因子

    std::vector<std::unique_ptr<Node<K, V>>> nodes_storage;
    std::mt19937 rng;
    std::uniform_real_distribution<float> dist;

    /**
     * @brief 随机生成节点的层数
     * 
     * 使用几何分布逻辑：每一层晋升到更高一层的概率为 p。
     * 
     * @return int 生成的随机层数，范围在 [1, max_level]
     */
    int random_level() {
        int level = 1;
        while (dist(rng) < p && level < max_level) {
            level++;
        }
        return level;
    }

public:
    /**
     * @brief SkipList 构造函数
     * @param max_lvl 最大允许层数
     * @param prob 晋升概率因子，默认 0.5
     */
    SkipList(int max_lvl, float prob = 0.5f)
        : max_level(max_lvl),
          current_level(1),
          p(prob),
          rng(std::random_device{}()),
          dist(0.0f, 1.0f) {
        // 创建哨兵头节点，其高度为最大层数
        auto head_node = std::make_unique<Node<K, V>>(K(), V(), max_level);
        head = head_node.get();
        nodes_storage.push_back(std::move(head_node));
    }

    /**
     * @brief 析构函数，负责释放跳表中所有节点的内存
     */
    ~SkipList() = default;

    /**
     * @brief 插入或更新一个键值对
     * 
     * @param key 待插入的键
     * @param value 待插入的值
     * @return true 插入成功
     * @return false 插入失败
     */
    bool insert(K key, V value);

    /**
     * @brief 根据键查找对应的值
     * 
     * @param key 待查找的键
     * @return std::optional<V> 如果找到则返回包含值的 optional，否则返回 std::nullopt
     */
    std::optional<V> search(K key);

    /**
     * @brief 根据键删除节点
     * 
     * @param key 待删除的键
     * @return true 删除成功
     * @return false 键不存在，删除失败
     */
    bool remove(K key);
};