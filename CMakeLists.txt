cmake_minimum_required(VERSION 3.5)
project(cielmalloc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(cielmalloc INTERFACE
)

target_include_directories(cielmalloc INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)