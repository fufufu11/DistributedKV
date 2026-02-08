#include <iostream>
#include <string>
#include <cstdlib>
#include "kv_store.h"

static std::string get_arg_value(int argc, char** argv, const std::string& key,
                                 const std::string& default_value) {
    for (int i = 1; i + 1 < argc; ++i) {
        if (key == argv[i]) {
            return argv[i + 1];
        }
    }
    return default_value;
}

int main(int argc, char** argv) {
    const std::string data_dir = get_arg_value(argc, argv, "--data", "./data");
    const std::string mode = get_arg_value(argc, argv, "--mode", "write");

    if (mode == "write") {
        KVStore store(data_dir);
        store.put(1, "v1");
        store.put(2, "v2");
        store.del(2);
        std::cout << "get(1)=" << store.get(1).value_or("<missing>") << std::endl;
        std::cout << "get(2)=" << store.get(2).value_or("<missing>") << std::endl;
        return 0;
    }

    if (mode == "crash") {
        KVStore store(data_dir);
        store.put(10, "v10");
        store.put(11, "v11");
        std::cout << "crashing_after_wal_sync" << std::endl;
        std::abort();
    }

    if (mode == "read") {
        KVStore store(data_dir);
        std::cout << "get(10)=" << store.get(10).value_or("<missing>") << std::endl;
        std::cout << "get(11)=" << store.get(11).value_or("<missing>") << std::endl;
        return 0;
    }

    std::cout << "usage: DistributedKV_bin.exe [--data <dir>] [--mode write|crash|read]" << std::endl;
    return 2;
}
