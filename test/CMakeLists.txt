cmake_minimum_required(VERSION 3.12)
project(cielmalloc_test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE "Debug")

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        58d77fa8070e8cec2dc1ed015d66b454c8d78850 # release-1.12.1, last version supporting C++11
)
FetchContent_MakeAvailable(googletest)

add_executable(cielmalloc_test
    src/bits.cpp
    src/large_alloc.cpp
    src/medium_alloc.cpp
    src/medium_slab.cpp
    src/pagemap.cpp
    src/pool.cpp
    src/sizeclass.cpp
    src/small_alloc.cpp
)

include(GoogleTest)
gtest_discover_tests(cielmalloc_test)

target_link_libraries(cielmalloc_test PRIVATE cielmalloc GTest::gtest_main)

target_compile_options(cielmalloc_test PRIVATE -Wall -Wextra -Werror)
