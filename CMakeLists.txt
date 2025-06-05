cmake_minimum_required(VERSION 3.31)
project(EmptyProject)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(googletest)
add_executable(EmptyProject
        main.cpp
        src/Demo.cpp
        include/Demo.h
)

include_directories(include)

add_executable(tests
        tests/tests.cpp
        src/Demo.cpp
        include/Demo.h
)
target_link_libraries(tests gtest)