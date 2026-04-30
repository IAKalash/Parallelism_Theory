cmake_minimum_required(VERSION 3.10)

project(MatrixVectorThreads LANGUAGES CXX)

find_package(Threads REQUIRED)

add_executable(dgemv_threads dgemv_threads.cpp)
target_compile_features(dgemv_threads PRIVATE cxx_std_17)
target_link_libraries(dgemv_threads PRIVATE Threads::Threads)
