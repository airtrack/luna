cmake_minimum_required(VERSION 2.6)

project(luna)

set(CMAKE_CXX_FLAGS
    "-std=c++11 -Wall -Wextra -Wno-unused-parameter"
    )

add_subdirectory(luna)
add_subdirectory(unittests)
