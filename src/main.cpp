#include <iostream>
#include <string>
#include <vector>
#include "kv_store.h" // 切换为使用集成了 WAL 的 KVStore

int main() {
    std::cout << "=== DistributedKV Persistence Demo ===" << std::endl;
    
    // 1. 指定数据存储目录
    std::string data_dir = "data";
    
    try {
        // 2. 初始化 KVStore
        // 如果 data/wal.log 存在，会自动触发 Replay 恢复数据
        KVStore store(data_dir);
        
        // 3. 检查之前的数据是否存在（验证持久化）
        std::cout << "\n[Checking History]" << std::endl;
        auto val = store.get(1);
        if (val.has_value()) {
            std::cout << "Found Key 1 (Recovered): " << val.value() << std::endl;
        } else {
            std::cout << "Key 1 not found (Fresh start)." << std::endl;
        }

        // 4. 执行写入操作
        std::cout << "\n[Executing Writes]" << std::endl;
        
        std::cout << "Put(1, \"Distributed\")" << std::endl;
        store.put(1, "Distributed");
        
        std::cout << "Put(2, \"System\")" << std::endl;
        store.put(2, "System");

        std::cout << "Put(3, \"To_Be_Deleted\")" << std::endl;
        store.put(3, "To_Be_Deleted");

        std::cout << "Del(3)" << std::endl;
        store.del(3);

        // 5. 即时查询验证
        std::cout << "\n[Current State Query]" << std::endl;
        auto v1 = store.get(1);
        auto v2 = store.get(2);
        auto v3 = store.get(3);

        std::cout << "Key 1: " << (v1.has_value() ? v1.value() : "null") << std::endl;
        std::cout << "Key 2: " << (v2.has_value() ? v2.value() : "null") << std::endl;
        std::cout << "Key 3: " << (v3.has_value() ? v3.value() : "null") << " (Should be null)" << std::endl;

        std::cout << "\n[Info] Data has been persisted to '" << data_dir << "/wal.log'" << std::endl;
        std::cout << "[Info] Try restarting this program to see data recovery!" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
