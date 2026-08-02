add_library(re_core
  core/log.cpp
  config/config.cpp
  accel/brute_accel.cpp
  film/film.cpp
  film/tonemap.cpp
  io/image_io.cpp
  render/cpu/cpu_debug_renderer.cpp
)
target_include_directories(re_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(re_core PUBLIC re_warnings tomlplusplus stb_image)

add_executable(re main.cpp)
target_link_libraries(re PRIVATE re_core)