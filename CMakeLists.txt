cmake_minimum_required(VERSION 3.31)

project(SiREN)

add_subdirectory(cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

find_package(GTest CONFIG REQUIRED)
find_package(protobuf CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS log)

include(GoogleTest)

add_subdirectory(src)
if(CMAKE_BUILD_TYPE STREQUAL Debug) 
    add_subdirectory(test)
endif()