# DistributedKV 测试记录文档

本文档专门用于记录项目的测试方法论、测试用例清单以及历史验收记录。

## 1. 测试体系概览 (GTest/CTest)

### 1.1 单元测试解决什么问题

单元测试（Unit Test）关注“一个很小的单元（函数/类/模块）在给定输入下是否输出符合预期”。它主要解决三类工程问题：
- **回归**：今天修好/写好的功能，明天改代码不会悄悄坏掉
- **定位**：出现 bug 时能快速把问题范围缩到某个模块或某条路径
- **重构保障**：你可以放心整理代码结构，只要测试还是绿的，就说明行为没变

### 1.2 本项目的测试入口

本项目的测试文件位于 `tests/` 目录：
- `tests/skiplist_test.cpp`：跳表核心逻辑测试（insert/search/remove）
- `tests/kv_store_test.cpp`：KVStore 整体集成测试（目录管理、WAL 检测、Put/Get 接口）
- `tests/wal_record_test.cpp`：WAL 记录编解码与 CRC32 校验测试

**运行方式**：
```powershell
# 方式 1: 使用 CTest (推荐)
ctest --test-dir build --output-on-failure

# 方式 2: 运行特定测试可执行文件
.\build\bin\skiplist_test.exe
```

### 1.3 写测试的 AAA 套路

把每个测试都按三段组织，读起来最清晰：
- **Arrange**：准备数据与环境（构造对象、准备输入）
- **Act**：执行动作（调用要测试的函数）
- **Assert**：断言结果（验证输出、状态、不变量）

### 1.4 让测试可复现

测试想要“稳定”，要尽量消除不确定性：
- **固定随机种子**：例如 `std::mt19937 rng(12345)`，确保 shuffle 结果固定
- **控制随机层数路径**：用 `prob=0.0f` 或 `prob=1.0f` 覆盖确定路径
- **避免依赖遍历顺序**：`unordered_map` 的遍历顺序不保证稳定，不要把“遍历顺序”作为测试依据

---

## 2. 测试用例清单与验收标准

### 2.1 SkipList 核心测试 (`tests/skiplist_test.cpp`)

覆盖跳表数据结构的核心 CRUD 操作：
- **空表行为**：查找/删除空表应返回未命中/false。
- **基础插入查询**：单条、批量、乱序插入后，查询结果应正确。
- **更新语义**：重复 Key 插入应覆盖 Value。
- **删除逻辑**：
    - 删除存在的 Key：后续不可查。
    - 删除不存在的 Key：返回 false，不破坏结构。
    - 删除边界 Key：最小/最大 Key 删除后结构仍完整。
- **极端层数**：在 `prob=1.0` (全满层) 和 `prob=0.0` (单层) 下逻辑均正确。

### 2.2 KVStore 集成测试 (`tests/kv_store_test.cpp`)

覆盖 KVStore 对外的集成行为（Task 1）：
- **目录管理**：自动创建 data 目录。
- **WAL 检测**：启动时能识别已存在的 `wal.log`。
- **接口打通**：`Put`/`Get`/`Delete` 能正确透传给底层的 SkipList。

### 2.3 WAL 编解码测试 (`tests/wal_record_test.cpp`)

覆盖持久化格式的底层逻辑 (Task 2)：
- **CRC32**：验证标准向量计算正确性。
- **Encode**：验证 `Put`/`Delete` 记录编码后的字节流长度、Header 字段及 Payload 内容。
- **Checksum**：验证编码中写入的 Checksum 与实际计算一致。

### 2.4 WAL 重放恢复测试 (`tests/kv_store_test.cpp`)

覆盖崩溃恢复场景 (Task 4) 以及崩溃模拟用例 (第 3 周 Task 6)：
- **NormalRecovery**：写入并销毁对象后，重建对象能恢复之前的数据（Put/Delete）。
- **TruncatedWAL**：WAL 尾部存在不完整记录（模拟断电半写），能自动忽略并恢复有效前缀。
- **CorruptedWAL**：WAL 中间数据损坏（Checksum 不匹配），能识别并停止重放（Fail-Stop）。
- **BulkRecovery1000**：写入 1000 条 Put，模拟重启后全部可读。
- **MixedPutDelRecovery**：Put/Delete/覆盖更新混合写入，模拟重启后最终状态正确。
- **TruncateMidRecord**：将 WAL 截断到某条记录中间，重启仅恢复完整前缀且不崩溃。
- **CorruptMiddleRecordStopsAtPrefix**：中间记录损坏时，重启仅恢复损坏记录之前的前缀并停止。

---

## 3. 历史测试记录

### 第 1-2 周：SkipList 与基础架构

- **2026-01-31**：SkipList 核心功能验收通过
    - 结果：`14/14 tests passed`
    - 范围：覆盖 Insert/Search/Remove 及边界条件。

- **2026-01-31**：KVStore 骨架验收通过 (Task 1)
    - 结果：`19/19 tests passed` (含 SkipList 14 个)
    - 新增用例：`CreatesDataDirectory`, `DetectsExistingWAL`, `BasicPutGet` 等。

### 第 3 周：WAL 预写日志

- **2026-02-04**：WAL 编解码与校验验收通过 (Task 2)
    - 结果：`22/22 tests passed` (全量)
    - 新增用例：`CRC32Test`, `WALRecordTest` (EncodePut/EncodeDelete)。

- **2026-02-05**：WAL 写入与持久化验收通过 (Task 3)
    - 结果：`23/23 tests passed`
    - 新增用例：`KVStoreTest.WALPersistenceCheck`。
    - 验证点：`Put`/`Delete` 操作后，WAL 文件被创建且包含预期数据（强持久化生效）。

- **2026-02-07**：WAL 读取与重放验收通过 (Task 4)
    - 结果：`26/26 tests passed`
    - 新增用例：`WALReplayTest` (`NormalRecovery`, `TruncatedWAL`, `CorruptedWAL`)。
    - 验证点：成功实现 Read -> Verify -> Apply 闭环；对崩溃截断和数据损坏有正确的容错处理。

- **2026-02-08**：崩溃模拟用例补齐验收通过 (Task 6)
    - 结果：`30/30 tests passed`
    - 新增用例：`WALReplayTest` (`BulkRecovery1000`, `MixedPutDelRecovery`, `TruncateMidRecord`, `CorruptMiddleRecordStopsAtPrefix`)。
    - 验证点：覆盖大批量重启恢复、Put/Delete 混合恢复、记录中途截断恢复前缀、以及中间记录损坏 fail-stop 行为。
