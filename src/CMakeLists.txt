add_library(re_core
  core/log.cpp
  core/color.cpp
  config/config.cpp
  accel/embree_accel.cpp
  accel/brute_accel.cpp
  film/film.cpp
  film/tonemap.cpp
  io/image_io.cpp
  texture/texture_build.cpp
  io/gltf/loader.cpp
  io/gltf/mesh.cpp
  io/gltf/accessor.cpp
  io/gltf/instancing.cpp
  io/gltf/texture.cpp
  io/gltf/material.cpp
  io/gltf/helpers/mesh_tangents.cpp
  render/cpu/cpu_debug_renderer.cpp
  render/cpu/cpu_path_renderer.cpp
  texture/texture_cache.cpp
  driver/offline_driver.cpp
  scene/scene_builder.cpp
  scene/mesh.cpp
  light/cdf_light_build.cpp
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
  mikktspace
)

add_executable(re main.cpp)
target_link_libraries(re PRIVATE re_core)