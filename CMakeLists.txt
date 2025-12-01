cmake_minimum_required(VERSION 3.31)

project(SiREN)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS )

add_subdirectory(src)
add_subdirectory(test)