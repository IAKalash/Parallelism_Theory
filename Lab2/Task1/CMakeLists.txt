cmake_minimum_required(VERSION 3.10)

project(MatrixVectorOMP)

find_package(OpenMP REQUIRED)

add_executable(dgemv_omp dgemv_omp.c)
target_link_libraries(dgemv_omp PRIVATE OpenMP::OpenMP_C)