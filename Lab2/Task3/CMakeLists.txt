cmake_minimum_required(VERSION 3.10)
project(Lab2_Task3)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(OpenMP REQUIRED)

add_executable(task3 main.cpp)
add_executable(task3_schedule_test main_schedule_test.cpp)

if(OpenMP_CXX_FOUND)
    target_link_libraries(task3 PUBLIC OpenMP::OpenMP_CXX)
    target_link_libraries(task3_schedule_test PUBLIC OpenMP::OpenMP_CXX)
endif()
