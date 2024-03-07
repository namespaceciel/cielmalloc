cmake_minimum_required(VERSION 3.5)
project(cielmalloc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(CIELMALLOC_BUILD_TESTS "Build unit tests" ${PROJECT_IS_TOP_LEVEL})

add_library(cielmalloc STATIC
        src/malloc.cpp
        src/treiber_stack.cpp
        src/thread_alloc.cpp
        src/span.cpp
        src/message_queue.cpp
        src/sizeclass.cpp
        src/headquarter_alloc.cpp
        src/system.cpp
        src/remote_request.cpp
        src/config.cpp
        src/thread_alloc_core.cpp
)

target_include_directories(cielmalloc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (CIELMALLOC_BUILD_TESTS)
    enable_testing()

    add_executable(cielmalloc_test
            test/test.cpp
    )

    add_subdirectory(third_party/googletest)

    target_link_libraries(cielmalloc_test PUBLIC
            cielmalloc
            GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(cielmalloc_test)

    target_compile_options(cielmalloc_test PUBLIC -Wall -Wextra -Werror)
endif ()