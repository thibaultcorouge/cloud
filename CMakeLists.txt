cmake_minimum_required(VERSION 3.15)
project(Cloud VERSION 1.0 LANGUAGES C)


set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)

file(GLOB_RECURSE SOURCES "src/*.c")

add_executable(cloud ${SOURCES})

target_compile_options(cloud PRIVATE -Wall -Wextra)

target_include_directories(cloud PRIVATE include)

target_link_libraries(cloud PRIVATE CURL::libcurl)
