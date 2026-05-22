cmake_minimum_required(VERSION 3.20)
project(comp8 CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(comp8 main.cpp)
target_compile_options(comp8 PRIVATE -Wall -Wextra -O2)

find_package(Catch2 3 REQUIRED)

enable_testing()
include(CTest)
include(Catch)

add_executable(comp8_tests tests.cpp)
target_link_libraries(comp8_tests PRIVATE Catch2::Catch2WithMain)
target_compile_options(comp8_tests PRIVATE -Wall -Wextra -O2)

catch_discover_tests(comp8_tests)
