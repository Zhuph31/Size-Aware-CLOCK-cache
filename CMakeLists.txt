cmake_minimum_required(VERSION 3.0)

project(lru_cache)

find_package(gflags REQUIRED)
include_directories(${gflags_INCLUDE_DIR})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "./*.cpp")
# add_executable(force_calculator main.cpp ${SOURCES})
add_executable(lru_cache lru_test.cpp utils.cpp)

add_compile_options(-Wno-sign-compare)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

target_link_libraries(lru_cache PUBLIC gflags)

