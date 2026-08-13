cmake_minimum_required(VERSION 3.20)
project(re LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)

add_library(re_warnings INTERFACE)
target_compile_options(re_warnings INTERFACE
  -Wall
  -Wextra
  -Wpedantic
  -Wshadow
  -Wconversion
)

enable_testing()

# external first: src and tests both consume targets defined there.
add_subdirectory(external)
add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(tools)