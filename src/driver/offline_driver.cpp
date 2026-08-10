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
#include "scene/scene_builder.h"

namespace re {

bool run_offline(const OfflineSettings& settings) {
  const Scene scene = make_cornell_scene();
  BruteAccel accel;
  accel.build(scene.geometry());

  const PinholeParams camera = make_pinhole(float3{0.0f, 0.0f, 1.08f},
                                            float3{0.0f, 0.0f, 0.0f},
                                            float3{0.0f, 1.0f, 0.0f},
                                            38.0f, settings.width, settings.height);
  Film film(settings.width, settings.height);
  const CpuDebugRenderer renderer(scene.geometry(), accel, DebugMode::Normals);
  const auto t0 = std::chrono::steady_clock::now();
  renderer.render(camera, film);
  const auto t1 = std::chrono::steady_clock::now();

  const double seconds = std::chrono::duration<double>(t1 - t0).count();
  log_info("Rendered {}x{} in {} seconds.", settings.width, settings.height, seconds);

  const std::vector<uint8_t> bytes = encode_srgb8(film);
  return write_png(settings.output_path, bytes.data(), film.width(), film.height());
}

}