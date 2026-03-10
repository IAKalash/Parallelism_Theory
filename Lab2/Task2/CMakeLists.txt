cmake_minimum_required(VERSION 3.10)

project(IntegralOMP)

find_package(OpenMP REQUIRED)

add_executable(integral_omp integral_omp.c)
target_link_libraries(integral_omp PRIVATE OpenMP::OpenMP_C m)
