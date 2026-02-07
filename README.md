# DistributedKV: 从零手写分布式 KV 存储引擎

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)]()
[![License](https://img.shields.io/badge/license-MIT-blue)]()
[![C++](https://img.shields.io/badge/C%2B%2B-20-blue.svg)]()

> **⚠️ Note**: 本项目目前处于开发初期（WIP），是一个用于学习分布式系统核心原理（LSM-Tree, WAL, Raft）的教学型项目。

## 📖 项目愿景

DistributedKV 旨在通过“手写核心组件”的方式，深度解析现代分布式数据库的底层原理。我们不追求极致的工业级性能，而是追求**代码的可读性**、**架构的清晰性**以及**核心机制的正确性**。

核心技术栈：
- **存储引擎**: LSM-Tree (Log-Structured Merge-Tree)
- **持久化**: Write-Ahead Log (WAL) with Strong Durability (fsync/_commit)
- **一致性**: Raft Consensus Algorithm (Planned)
- **语言标准**: Modern C++20

## 🚀 当前进度 (Status)

| 模块 | 功能 | 状态 | 说明 |
| :--- | :--- | :--- | :--- |
| **MemTable** | SkipList (跳表) | ✅ 完成 | 支持 Insert/Search/Remove，基于随机层数优化 |
| **Storage** | KVStore 骨架 | ✅ 完成 | 统一管理内存与磁盘资源，提供对外接口 |
| **Persistence** | WAL Writer | ✅ 完成 | 实现 `Write -> Flush -> Sync` 强持久化链路 |
| **Persistence** | WAL Reader | ✅ 完成 | 实现基于 Checksum 的崩溃恢复 (Crash Recovery) |
| **Storage** | SSTable | 📅 计划中 | 磁盘有序文件与布隆过滤器 |
| **Consensus** | Raft | 📅 计划中 | Leader 选举与日志复制 |

详细的学习路径与任务拆解请参考：[📚 项目学习手册 (Learning Manual)](docs/DistributedKV_Guide/Learning_Manual.md)

## 🛠️ 快速上手 (Quick Start)

### 环境要求
- **编译器**: GCC 10+ / MSVC 19.28+ (需支持 C++20)
- **构建工具**: CMake 3.15+
- **平台**: Windows (推荐使用 MinGW-w64) / Linux

### 构建与运行 (Windows PowerShell)

本项目提供了便捷的构建脚本 `build.ps1`，自动处理 CMake 配置与编译。

```powershell
# 1. 克隆仓库
git clone https://github.com/YourUsername/DistributedKV.git
cd DistributedKV

# 2. 一键编译
.\build.ps1

# 3. 运行单元测试 (推荐)
ctest --test-dir build --output-on-failure

# 4. 运行持久化 Demo 程序
.\build\bin\DistributedKV_bin.exe
```

### � 性能基准测试 (Benchmarks)

项目包含一个针对 SkipList 的基准测试工具，可用于评估不同参数下的性能表现。

```powershell
# 运行基准测试 (默认参数: n=100000)
.\build\bin\benchmark_skiplist.exe

# 自定义参数运行
# --n: 键值对数量
# --reads: 读取次数
# --max-level: 跳表最大层数
.\build\bin\benchmark_skiplist.exe --n 1000000 --reads 500000 --max-level 20
```

## �📂 目录结构

```text
DistributedKV/
├── docs/               # 文档中心
│   ├── DistributedKV_Guide/
│   │   ├── Learning_Manual.md  # 核心学习手册（原理+任务）
│   │   └── Test_Record.md      # 测试验收记录
├── examples/           # 示例与基准测试代码
│   └── benchmark_skiplist.cpp
├── include/            # 头文件 (API 定义)
│   ├── kv_store.h      # 存储引擎入口 (含 WAL 恢复逻辑)
│   ├── skiplist.h      # 跳表实现
│   └── wal_record.h    # WAL 格式定义
├── src/                # 源代码
│   └── main.cpp        # 演示程序入口
├── tests/              # 单元测试 (GTest)
└── build.ps1           # 构建脚本
```

## 🧪 测试策略

本项目采用 **TDD (测试驱动开发)** 模式，确保每个核心组件都经过严格的单元测试验证。
- **SkipList**: 覆盖并发插入、删除边界、随机层数分布。
- **WAL**: 验证编码正确性、Checksum 校验以及**断电持久化能力**。
- **Recovery**: 模拟数据截断与损坏，验证系统恢复的健壮性。

查看详细测试记录：[Test_Record.md](docs/DistributedKV_Guide/Test_Record.md)

## 🤝 贡献与反馈

这是一个开放的学习项目，欢迎提交 Issue 或 Pull Request 来改进代码或补充文档。

---
*Happy Coding!* 🚀
