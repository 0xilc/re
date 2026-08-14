#include "driver/offline_driver.h"
#include <chrono>
#include <vector>
#include "accel/embree_accel.h"
#include "camera/pinhole.h"
#include "config/render_settings.h"
#include "core/log.h"
#include "film/film.h"
#include "film/tonemap.h"
#include "io/image_io.h"
#include "render/cpu/cpu_debug_renderer.h"
#include "render/cpu/cpu_path_renderer.h"
#include "scene/scene_builder.h"
#include "scene/scene_view.h"
#include "parallel/thread_pool.h"


namespace re {

bool run_offline(const RenderSettings& settings) {
  const Scene scene = make_cornell_scene();

  EmbreeAccel accel;
  accel.build(scene.geometry());

  SceneView scene_view;
  scene_view.geometry       = scene.geometry();
  scene_view.materials      = scene.material_table();
  scene_view.material_count = static_cast<int>(scene.materials.size());

  ThreadPool pool = ThreadPool(settings.threads);
  
  const PinholeParams camera = make_pinhole(float3{0.0f, 0.0f, 1.08f},
                                            float3{0.0f, 0.0f, 0.0f},
                                            float3{0.0f, 1.0f, 0.0f},
                                            45.0f, settings.width, settings.height);
  const CpuServices services(accel);
  const CpuPathRenderer path_renderer(
    scene_view, scene.light_view(), services, pool, settings.path_tracer);

  Film film(settings.width, settings.height);
  const auto t0 = std::chrono::steady_clock::now();
  path_renderer.render(camera, film);
  const auto t1 = std::chrono::steady_clock::now();

  const double seconds = std::chrono::duration<double>(t1 - t0).count();
  log_info("Rendered {}x{} in {} seconds.", settings.width, settings.height, seconds);

  const std::vector<uint8_t> bytes = encode_srgb8(film);
  if (film.nonfinite_count() > 0) {
    log_error("{} non-finite samples were dropped; this image is not trustworthy", film.nonfinite_count());
  }

  return write_png(settings.output_path, bytes.data(), film.width(), film.height());
}

}