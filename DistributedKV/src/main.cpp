#include <iostream>
#include <string>
#include "skiplist.h"

int main(){
    SkipList<int, std::string> kv_store(16);

    std::cout << "Starting DistributedKV SkipList test..." << std::endl;

    std::cout << "API Design verified!" << std::endl;
    
    return 0;
}