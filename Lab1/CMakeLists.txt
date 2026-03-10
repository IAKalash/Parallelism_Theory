cmake_minimum_required(VERSION 3.10)

project(SinusSum)

option(FLOAT "Build with float instead of double" OFF)

if(FLOAT)
    message(STATUS "Building with FLOAT")
    add_definitions(-DTYPE=float)
else()
    message(STATUS "Building with DOUBLE")
    add_definitions(-DTYPE=double)
endif()

add_executable(main array.cpp)