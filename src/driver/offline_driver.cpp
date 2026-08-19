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
#include "parallel/thread_pool.h"
#include "render/cpu/cpu_path_renderer.h"
#include "render/cpu/cpu_debug_renderer.h"
#include "scene/scene_builder.h"
#include "scene/scene_view.h"
#include "io/gltf_loader.h"

namespace re {

bool run_offline(const RenderSettings& settings) {
    Scene scene;
    if (!load_gltf("data/SimpleInstancing.glb", scene)) {
      log_error("driver: could not load scene");
      return false;
    }
    
    const uint16_t lamp = scene.add_material(
        make_material({0.0f, 0.0f, 0.0f}, 0.0f, 1.0f, {30.0f, 30.0f, 30.0f}));
    scene.add_quad({-1, 3, -1}, {1, 3, -1}, {1, 3, 1}, {-1, 3, 1}, lamp);

    scene.finalize();

    EmbreeAccel accel;
    accel.build(scene.geometry());

    SceneView scene_view;
    scene_view.geometry       = scene.geometry();
    scene_view.materials      = scene.material_table();
    scene_view.material_count = static_cast<int>(scene.materials.size());

    ThreadPool pool(settings.threads);
    log_info("using {} threads", pool.thead_count());

    const PinholeParams camera =
        make_pinhole(float3{0.0f, 0.0f, 7.08f},
                     float3{0.0f, 0.0f, 0.0f},
                     float3{0.0f, 1.0f, 0.0f},
                     45.0f, settings.width, settings.height);

    const CpuServices     services(accel);
    // const CpuPathRenderer path_renderer(scene_view, scene.light_view(), services, pool,
    //                                     settings.path_tracer);
    const CpuDebugRenderer debug_renderer(scene_view.geometry, accel, DebugMode::Normals, pool);
    Film film(settings.width, settings.height);

    const auto t0 = std::chrono::steady_clock::now();
    debug_renderer.render(camera, film);
    const auto t1 = std::chrono::steady_clock::now();

    log_info("Rendered {}x{} in {:.3f} seconds.", settings.width, settings.height,
             std::chrono::duration<double>(t1 - t0).count());

    if (film.nonfinite_count() > 0) {
        log_error("{} non-finite samples were dropped; this image is not trustworthy",
                  film.nonfinite_count());
    }

    const std::vector<float3>  mapped = tonemap(film, settings.tonemap);
    const std::vector<uint8_t> bytes  = encode_srgb8(mapped);

    return write_png(settings.output_path, bytes.data(), film.width(), film.height());
}

}  // namespace re