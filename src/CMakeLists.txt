add_library(re_core
  core/log.cpp
  config/config.cpp
  accel/embree_accel.cpp
  accel/brute_accel.cpp
  film/film.cpp
  film/tonemap.cpp
  io/image_io.cpp
  io/gltf/loader.cpp
  io/gltf/mesh.cpp
  io/gltf/accessor.cpp
  io/gltf/instancing.cpp
  render/cpu/cpu_debug_renderer.cpp
  render/cpu/cpu_path_renderer.cpp
  driver/offline_driver.cpp
  scene/scene_builder.cpp
  scene/mesh.cpp
  parallel/thread_pool.cpp
  parallel/scheduler.cpp
)

target_include_directories(re_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(re_core PUBLIC
  re_warnings
  tomlplusplus
  stb_image
  embree
  cgltf
)

add_executable(re main.cpp)
target_link_libraries(re PRIVATE re_core)