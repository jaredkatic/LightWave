cmake_minimum_required(VERSION 3.14)
project(Lightwave VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# Include project headers
include_directories(include)

# 1. Build Main Executable
add_executable(lightwave src/main.cpp)

# 2. Fetch GoogleTest automatically
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# For Windows MSVC runtime compatibility
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# 3. Build & Link Tests
enable_testing()

add_executable(lightwave_tests tests/test_circuit_breaker.cpp)
target_link_libraries(lightwave_tests PRIVATE gtest_main)

include(GoogleTest)
gtest_discover_tests(lightwave_tests)
