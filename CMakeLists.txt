cmake_minimum_required(VERSION 3.12)
project(cielmalloc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose build type: Debug/Release/RelWithDebInfo/MinSizeRel." FORCE)
endif (NOT CMAKE_BUILD_TYPE)

set(FETCHCONTENT_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party")

include(FetchContent)
FetchContent_Declare(
    ciellab
    GIT_REPOSITORY https://github.com/namespaceciel/ciellab.git
    GIT_TAG        43379267997d4c6e9ac38c7ebc3580e9157c666f
)
FetchContent_MakeAvailable(ciellab)

option(CIELMALLOC_BUILD_TESTS "Build unit tests" ${PROJECT_IS_TOP_LEVEL})

add_library(cielmalloc INTERFACE)

target_link_libraries(cielmalloc INTERFACE ciellab)

target_include_directories(cielmalloc INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (CIELMALLOC_BUILD_TESTS)
	enable_testing()
	add_subdirectory(test)
endif ()
