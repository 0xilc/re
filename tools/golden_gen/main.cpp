#include <cstdio>
#include <string>
#include <vector>

#include "accel/embree_accel.h"
#include "camera/pinhole.h"
#include "config/render_settings.h"
#include "film/film.h"
#include "film/tonemap.h"
#include "io/image_io.h"
#include "parallel/thread_pool.h"
#include "render/cpu/cpu_path_renderer.h"
#include "scene/scene_builder.h"
#include "scene/scene_view.h"

namespace {
constexpr int         kWidth    = 128;
constexpr int         kHeight   = 128;
constexpr int         kSpp      = 32;
constexpr int         kMaxDepth = 8;
constexpr re::Tonemap kTonemap  = re::Tonemap::Reinhard;
}  // namespace

int main(int argc, char** argv) {
    const std::string out_path =
        argc > 1 ? argv[1] : "tests/golden/cornell/reference.png";

    const re::Scene scene = re::make_cornell_scene();

    re::EmbreeAccel accel;
    accel.build(scene.geometry());

    re::SceneView view;
    view.geometry       = scene.geometry();
    view.materials      = scene.material_table();
    view.material_count = static_cast<int>(scene.materials.size());

    re::ThreadPool pool(0);

    const re::PinholeParams camera = re::make_pinhole(
        {0.0f, 0.0f, 1.08f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 38.0f, kWidth, kHeight);

    const re::CpuServices services(accel);

    re::PathTracerSettings pt;
    pt.spp        = kSpp;
    pt.max_depth  = kMaxDepth;
    pt.integrator = re::IntegratorMode::Mis;

    const re::CpuPathRenderer renderer(view, scene.light_view(), services, pool, pt);

    re::Film film(kWidth, kHeight);
    renderer.render(camera, film);

    if (film.nonfinite_count() > 0) {
        std::printf("REFUSING: %d non-finite samples. Fix that before pinning a reference.\n",
                    film.nonfinite_count());
        return 1;
    }

    const std::vector<re::float3>  mapped = re::tonemap(film, kTonemap);
    const std::vector<uint8_t>     bytes  = re::encode_srgb8(mapped);

    if (!re::write_png(out_path, bytes.data(), kWidth, kHeight)) return 1;

    std::printf("wrote %s (%dx%d, %d spp, depth %d)\n", out_path.c_str(), kWidth, kHeight, kSpp,
                kMaxDepth);
    return 0;
}