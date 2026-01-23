cmake_minimum_required(VERSION 3.10)
project(DistributedKV LANGUAGES CXX)

# 强制导出编译数据库 (用于编辑器代码跳转和错误检测)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 强制 C++20 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 纯 GCC/Clang 优化配置
add_compile_options(
    -Wall 
    -Wextra 
    -Wpedantic 
    -g 
    -O2 
    -march=native # 针对本地 CPU 进行优化
)

# 包含头文件
include_directories(${PROJECT_SOURCE_DIR}/include)

# 添加核心可执行文件
add_executable(DistributedKV_bin src/main.cpp)

# 打印构建环境信息
message(STATUS "Build System: Pure GCC/MinGW Environment")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
