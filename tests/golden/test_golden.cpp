#include "doctest.h"

#include <cmath>
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

#include "stb_image.h"

namespace {

constexpr int         kWidth    = 128;
constexpr int         kHeight   = 128;
constexpr int         kSpp      = 32;
constexpr int         kMaxDepth = 8;
constexpr re::Tonemap kTonemap  = re::Tonemap::Reinhard;
constexpr double kRmseTolerance = 0.002;

const char* kReferencePath = "tests/golden/cornell/reference.png";

std::vector<uint8_t> render_cornell() {
    const re::Scene scene = re::make_cornell_scene();

    const re::SceneGeometryView geometry = scene.make_geometry_view();

    re::EmbreeAccel accel;
    accel.build(geometry);

    re::SceneView view;
    view.geometry       = geometry;
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

    REQUIRE(film.nonfinite_count() == 0);

    const std::vector<re::float3> mapped = re::tonemap(film, kTonemap);
    return re::encode_srgb8(mapped);
}

}  // namespace

TEST_CASE("golden: cornell matches the stored reference") {
    int            w = 0, h = 0, channels = 0;
    unsigned char* reference = stbi_load(kReferencePath, &w, &h, &channels, 3);

    if (reference == nullptr) {
        MESSAGE("no reference at " << kReferencePath
                                   << " -- generate one with: re_tests_golden --update");
        return;
    }

    REQUIRE(w == kWidth);
    REQUIRE(h == kHeight);

    const std::vector<uint8_t> rendered = render_cornell();
    REQUIRE(rendered.size() == static_cast<size_t>(w) * h * 3);

    double sum_sq   = 0.0;
    double max_diff = 0.0;
    int    max_x = 0, max_y = 0;

    for (size_t i = 0; i < rendered.size() / 3; ++i) {
        double pixel_sq = 0.0;
        for (int c = 0; c < 3; ++c) {
            const float la = re::srgb_to_linear(reference[i * 3 + c] / 255.0f);
            const float lb = re::srgb_to_linear(rendered[i * 3 + c] / 255.0f);
            const double d = static_cast<double>(la) - lb;
            pixel_sq += d * d;
        }
        sum_sq += pixel_sq;

        const double rms = std::sqrt(pixel_sq / 3.0);
        if (rms > max_diff) {
            max_diff = rms;
            max_x    = static_cast<int>(i % static_cast<size_t>(w));
            max_y    = static_cast<int>(i / static_cast<size_t>(w));
        }
    }

    stbi_image_free(reference);

    const double rmse = std::sqrt(sum_sq / static_cast<double>(rendered.size()));

    CAPTURE(rmse);
    CAPTURE(max_diff);
    CAPTURE(max_x);
    CAPTURE(max_y);

    if (rmse >= kRmseTolerance) {
        re::write_png("golden_failed.png", rendered.data(), w, h);
        MESSAGE("wrote golden_failed.png -- compare with: image_diff "
                << kReferencePath << " golden_failed.png diff.png");
    }

    CHECK(rmse < kRmseTolerance);
}