cmake_minimum_required(VERSION 3.0)
project(radon-tf LANGUAGES CXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_library(radon-tf
  src/radon.cpp
)

target_include_directories(radon-tf PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

find_package(Threads)
find_package(OpenCV REQUIRED)

add_executable(main
  src/main.cpp
)

target_link_libraries(main ${OpenCV_LIBRARIES} -pthread radon-tf)
