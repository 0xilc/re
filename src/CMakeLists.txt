add_library(re_core
  main.cpp
  core/log.cpp
  config/config.cpp
)
target_include_directories(re_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(re_core PUBLIC re_warnings tomlplusplus)

add_executable(re main.cpp)
target_link_libraries(re PRIVATE re_core)