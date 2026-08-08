#include "driver/offline_driver.h"
#include <chrono>
#include <vector>
#include "accel/brute_accel.h"
#include "camera/pinhole.h"
#include "core/log.h"
#include "film/film.h"
#include "film/tonemap.h"
#include "io/image_io.h"
#include "render/cpu/cpu_debug_renderer.h"
#include "scene/geometry.h"

namespace re {

namespace {
// --- A hardcoded basic scene ---
const std::vector<float3> positions = {
    {-4.0f, -1.0f, -1.0f},  // 0  floor, near-left
    { 4.0f, -1.0f, -1.0f},  // 1  floor, near-right
    { 4.0f, -1.0f, -9.0f},  // 2  floor, far-right
    {-4.0f, -1.0f, -9.0f},  // 3  floor, far-left
    {-1.0f, -0.5f, -3.0f},  // 4  triangle, left
    { 1.0f, -0.5f, -3.0f},  // 5  triangle, right
    { 0.0f,  1.0f, -3.0f},  // 6  triangle, apex
};
const std::vector<uint32_t> indices = {
    0, 1, 2,  // floor A
    0, 2, 3,  // floor B
    4, 5, 6,  // upright triangle
};
}

bool run_offline(const OfflineSettings& settings) {
  GeometryView geometry;
  geometry.positions = positions.data();
  geometry.indices = indices.data();
  geometry.vertex_count = static_cast<int>(positions.size());
  geometry.triangle_count = static_cast<int>(indices.size() / 3);

  BruteAccel accel;
  accel.build(geometry);

  const PinholeParams camera = make_pinhole(float3{0.0f, 0.0f, 0.0f},
                                            float3{0.0f, 0.0f, -1.0f},
                                            float3{0.0f, 1.0f, 0.0f},
                                            settings.fov_y_deg, settings.width, settings.height);
  Film film(settings.width, settings.height);
  const CpuDebugRenderer renderer(geometry, accel, DebugMode::Lambert);
  const auto t0 = std::chrono::steady_clock::now();
  renderer.render(camera, film);
  const auto t1 = std::chrono::steady_clock::now();

  const double seconds = std::chrono::duration<double>(t1 - t0).count();
  log_info("Rendered {}x{} in {} seconds.", settings.width, settings.height, seconds);

  const std::vector<uint8_t> bytes = encode_srgb8(film);
  return write_png(settings.output_path, bytes.data(), film.width(), film.height());
}

}