cmake_minimum_required(VERSION 3.31)
project(EmptyProject)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(googletest)

add_executable(EmptyProject
        include/Demo.h
        src/Demo.cpp
)
include_directories(include)

add_executable(tests
        tests/tests.cpp
        include/Demo.h
        src/Demo.cpp
)
target_link_libraries(tests gtest)