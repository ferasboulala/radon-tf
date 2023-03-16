cmake_minimum_required(VERSION 3.0)
project(radon-tf LANGUAGES CXX)

set(CMAKE_C_COMPILER "gcc-11")
set(CMAKE_CXX_COMPILER "g++-11")
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(ALL_LIBRARIES)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

add_compile_options(
  -Ofast
  -march=native
  -Wall
  -Wpedantic
  -Werror
  -std=c++2a
  -std=gnu++2a
  -fverbose-asm
  -save-temps
)

find_package(Threads REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(app)
add_subdirectory(benchmarks)