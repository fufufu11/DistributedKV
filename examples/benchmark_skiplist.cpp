#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <map>
#include <numeric>
#include <random>
#include <string>
#include <string_view>
#include <vector>

#include "skiplist.h"

struct Options {
    std::size_t n = 100000;
    std::size_t reads = 0;
    std::uint32_t seed = 12345;
    int max_level = 16;
    float p = 0.5f;
};

static bool starts_with(std::string_view s, std::string_view prefix) {
    return s.size() >= prefix.size() && s.substr(0, prefix.size()) == prefix;
}

static void print_usage(const char* prog) {
    std::cout
        << "Usage: " << prog << " [--n N] [--reads R] [--seed S] [--max-level L] [--p P]\n"
        << "  --n         number of unique keys (default: 100000)\n"
        << "  --reads     number of lookups (default: n)\n"
        << "  --seed      shuffle seed (default: 12345)\n"
        << "  --max-level skiplist max level (default: 16)\n"
        << "  --p         promotion probability (default: 0.5)\n";
}

static bool parse_args(int argc, char** argv, Options& opt) {
    auto parse_kv = [&](std::string_view key, std::string_view value) -> bool {
        try {
            if (key == "--n") {
                opt.n = static_cast<std::size_t>(std::stoull(std::string(value)));
                return true;
            }
            if (key == "--reads") {
                opt.reads = static_cast<std::size_t>(std::stoull(std::string(value)));
                return true;
            }
            if (key == "--seed") {
                opt.seed = static_cast<std::uint32_t>(std::stoul(std::string(value)));
                return true;
            }
            if (key == "--max-level") {
                opt.max_level = std::stoi(std::string(value));
                return true;
            }
            if (key == "--p") {
                opt.p = std::stof(std::string(value));
                return true;
            }
        } catch (...) {
            return false;
        }
        return false;
    };

    for (int i = 1; i < argc; ++i) {
        std::string_view arg(argv[i]);
        if (arg == "--help" || arg == "-h") return false;

        auto eq = arg.find('=');
        if (eq != std::string_view::npos) {
            std::string_view k = arg.substr(0, eq);
            std::string_view v = arg.substr(eq + 1);
            if (!parse_kv(k, v)) return false;
            continue;
        }

        if (starts_with(arg, "--")) {
            if (i + 1 >= argc) return false;
            std::string_view k = arg;
            std::string_view v(argv[++i]);
            if (!parse_kv(k, v)) return false;
            continue;
        }

        return false;
    }

    if (opt.reads == 0) opt.reads = opt.n;
    if (opt.n == 0) return false;
    if (opt.max_level <= 0) return false;
    if (!(opt.p > 0.0f && opt.p < 1.0f)) return false;
    return true;
}

template <class F>
static std::chrono::nanoseconds time_it(F&& f) {
    auto start = std::chrono::steady_clock::now();
    f();
    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
}

static double ns_per_op(std::chrono::nanoseconds ns, std::uint64_t ops) {
    if (ops == 0) return 0.0;
    return static_cast<double>(ns.count()) / static_cast<double>(ops);
}

static double ops_per_sec(std::chrono::nanoseconds ns, std::uint64_t ops) {
    if (ns.count() <= 0) return 0.0;
    return (static_cast<double>(ops) * 1e9) / static_cast<double>(ns.count());
}

static void print_result(std::string_view name, std::string_view phase, std::chrono::nanoseconds ns, std::uint64_t ops) {
    double ms = static_cast<double>(ns.count()) / 1e6;
    std::cout << name << " " << phase << ": " << ms << " ms"
              << " | " << ops_per_sec(ns, ops) << " ops/s"
              << " | " << ns_per_op(ns, ops) << " ns/op"
              << "\n";
}

int main(int argc, char** argv) {
    Options opt;
    if (!parse_args(argc, argv, opt)) {
        print_usage(argv[0]);
        return 2;
    }

    std::vector<int> keys(opt.n);
    std::iota(keys.begin(), keys.end(), 0);
    std::mt19937 rng(opt.seed);
    std::shuffle(keys.begin(), keys.end(), rng);

    std::cout << "Benchmark: SkipList vs std::map\n";
    std::cout << "n=" << opt.n << " reads=" << opt.reads << " seed=" << opt.seed
              << " max_level=" << opt.max_level << " p=" << opt.p << "\n";
    std::cout << "Note: build with Release/-O2 for meaningful numbers.\n\n";

    std::uint64_t checksum_skiplist = 0;
    std::uint64_t checksum_map = 0;

    SkipList<int, int> sl(opt.max_level, opt.p);
    auto sl_insert = time_it([&] {
        for (int k : keys) {
            sl.insert(k, k);
        }
    });

    auto sl_read = time_it([&] {
        for (std::size_t i = 0; i < opt.reads; ++i) {
            int k = keys[i % keys.size()];
            auto v = sl.search(k);
            checksum_skiplist += static_cast<std::uint64_t>(v.has_value() ? v.value() : 0);
        }
    });

    std::map<int, int> mp;
    auto mp_insert = time_it([&] {
        for (int k : keys) {
            mp.emplace(k, k);
        }
    });

    auto mp_read = time_it([&] {
        for (std::size_t i = 0; i < opt.reads; ++i) {
            int k = keys[i % keys.size()];
            auto it = mp.find(k);
            checksum_map += static_cast<std::uint64_t>(it != mp.end() ? it->second : 0);
        }
    });

    print_result("SkipList", "insert", sl_insert, static_cast<std::uint64_t>(opt.n));
    print_result("SkipList", "read  ", sl_read, static_cast<std::uint64_t>(opt.reads));
    std::cout << "SkipList checksum: " << checksum_skiplist << "\n\n";

    print_result("std::map", "insert", mp_insert, static_cast<std::uint64_t>(opt.n));
    print_result("std::map", "read  ", mp_read, static_cast<std::uint64_t>(opt.reads));
    std::cout << "std::map checksum: " << checksum_map << "\n\n";

    if (checksum_skiplist != checksum_map) {
        std::cerr << "checksum mismatch\n";
        return 1;
    }

    return 0;
}

