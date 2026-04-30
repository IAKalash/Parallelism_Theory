cmake_minimum_required(VERSION 3.10)
project(Lab3Task2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_executable(task2_server main.cpp)
target_link_libraries(task2_server PRIVATE Threads::Threads)

add_executable(task2_check test_check.cpp)
