#include <iostream>
#include <string>
#include "skiplist.h"

int main(){
    std::cout << "=== DistributedKV SkipList Demo ===" << std::endl;   

    // 1. 初始化跳表，最大层数为 6
    SkipList<int, std::string> kv_store(6);

    // 2. 插入数据
    std::cout << "\n[Insert Operations]" << std::endl; 
    kv_store.insert(1, "Alice");
    std::cout << "Inserted: {1, \"Alice\"}" << std::endl;
    kv_store.insert(3, "Bob"); 
    std::cout << "Inserted: {3, \"Bob\"}" << std::endl;
    kv_store.insert(2, "Charlie");
    std::cout << "Inserted: {2, \"Charlie\"}" << std::endl;

    // 3. 查找存在的 Key
    std::cout << "\n[Search Operations]" << std::endl;
    auto result = kv_store.search(1);
    if(result.has_value()){
        std::cout << "Found Key 1: " << result.value() << std::endl;
    } else {
        std::cout << "Key 1 not found." << std::endl;
    }

    // 4. 查找不存在的 Key
    result = kv_store.search(99);
    if(result.has_value()){
        std::cout << "Found Key 99: " << result.value() << std::endl;
    } else {
        std::cout << "Key 99 not found (As expected)." << std::endl;
    }

    // 5. 更新数据
    std::cout << "\n[Update Operation]" << std::endl;
    kv_store.insert(1, "Alice_Updated");
    std::cout << "Updated Key 1 to \"Alice_Updated\"" << std::endl;
    result = kv_store.search(1);
    if(result.has_value()){
        std::cout << "Found Key 1: " << result.value() << std::endl;
    }

    std::cout << "\n=== Demo Finished ===" << std::endl;
    return 0;
}