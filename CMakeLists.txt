cmake_minimum_required(VERSION 3.15)
project(Cloud VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(cloud ${SOURCES})

target_include_directories(cloud PRIVATE include)

target_link_libraries(cloud PRIVATE CURL::libcurl)
