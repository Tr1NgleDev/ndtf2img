cmake_minimum_required(VERSION 3.10)
project(ndtf2img)

add_subdirectory("libs/ndtf")

set(CMAKE_CXX_STANDARD 20)

add_executable(ndtf2img main.cpp)

target_link_libraries(ndtf2img PRIVATE ndtf)
