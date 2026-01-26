cmake_minimum_required(VERSION 3.14)
project(DistributedKV LANGUAGES CXX)

# --- 全局设置 ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- 编译选项优化 ---
if(MSVC)
    add_compile_options(/W4 /Zc:__cplusplus /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O2 -g)
endif()

# --- Google Test 接入 (使用固定版本以提高稳定性) ---
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# 对于 Windows 环境，防止 GTest 修改运行时库选择 (MT/MD)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# --- 核心库 ---
# 建议将核心逻辑定义为库，方便主程序和测试共用
add_library(DistributedKV_lib INTERFACE)
target_include_directories(DistributedKV_lib INTERFACE ${PROJECT_SOURCE_DIR}/include)

# --- 可执行程序 ---
add_executable(DistributedKV_bin src/main.cpp)
target_link_libraries(DistributedKV_bin PRIVATE DistributedKV_lib)

# --- 单元测试 ---
add_executable(skiplist_test tests/skiplist_test.cpp)
target_link_libraries(skiplist_test PRIVATE DistributedKV_lib GTest::gtest_main)

# 让 ctest 能够识别到此测试
include(GoogleTest)
gtest_discover_tests(skiplist_test)

# --- 打印状态 ---
message(STATUS "--------------------------------------------")
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "--------------------------------------------")
