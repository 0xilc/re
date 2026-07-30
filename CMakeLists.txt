cmake_minimum_required(VERSION 3.20)
project(re LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(re_warnings INTERFACE)
target_compile_options(re_warnings INTERFACE
  -Wall
  -Wextra
  -Wpedantic
  -Wshadow
  -Wconversion
)

add_subdirectory(external)
add_subdirectory(src)
add_subdirectory(tests)