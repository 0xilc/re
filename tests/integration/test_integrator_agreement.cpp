#include "doctest.h"

#include <cmath>
#include <string>

#include "accel/embree_accel.h"
#include "camera/pinhole.h"
#include "core/sampler.h"
#include "integrator/path_integrator.h"
#include "light/cdf_light_build.h"
#include "render/cpu/cpu_services.h"
#include "render/cpu/texture_cache.h"
#include "shading/ray_cone.h"
#include "scene/scene_builder.h"

// Naive, NEE and MIS are three estimators of the same integral. If a pdf, a
// pick probability or a MIS weight is wrong they still produce a plausible
// image, they just converge somewhere else. Comparing their means is what
// catches that.
namespace {

constexpr int kResolution = 24;
constexpr int kSpp        = 400;
constexpr int kMaxDepth   = 6;

// A box lit by its whole ceiling. Cornell's small lamp would leave naive far
// too noisy to compare against at any spp this test can afford.
re::Scene make_bright_box() {
  re::Scene scene;

  constexpr float h = 0.5f;

  const uint16_t white = scene.add_material(re::make_material({0.73f, 0.73f, 0.73f}, 0.0f, 1.0f));
  const uint16_t red   = scene.add_material(re::make_material({0.65f, 0.05f, 0.05f}, 0.0f, 1.0f));
  const uint16_t green = scene.add_material(re::make_material({0.12f, 0.45f, 0.15f}, 0.0f, 1.0f));
  const uint16_t lamp  = scene.add_material(
      re::make_material({0.0f, 0.0f, 0.0f}, 0.0f, 1.0f, {4.0f, 4.0f, 4.0f}));

  scene.add_quad({-h, -h, h}, {h, -h, h}, {h, -h, -h}, {-h, -h, -h}, white);   // floor
  scene.add_quad({-h, -h, -h}, {h, -h, -h}, {h, h, -h}, {-h, h, -h}, white);   // back
  scene.add_quad({-h, -h, -h}, {-h, h, -h}, {-h, h, h}, {-h, -h, h}, red);     // left
  scene.add_quad({h, -h, -h}, {h, -h, h}, {h, h, h}, {h, h, -h}, green);       // right
  scene.add_quad({-h, h, -h}, {h, h, -h}, {h, h, h}, {-h, h, h}, lamp);        // ceiling

  return scene;
}

struct Rendered {
  re::Spectrum mean;
  double       luminance;
};

template <typename Li>
Rendered render_mean(const re::PinholeParams& camera, Li&& li) {
  re::Spectrum sum = re::make_empty_spectrum();

  for (int y = 0; y < kResolution; ++y) {
    for (int x = 0; x < kResolution; ++x) {
      for (int s = 0; s < kSpp; ++s) {
        re::Sampler sampler(x, y, s);
        const re::Ray ray = re::generate_ray(camera, x, y, sampler.next_2d());
        sum = sum + li(ray, sampler);
      }
    }
  }

  const float inv = 1.0f / static_cast<float>(kResolution * kResolution * kSpp);
  const re::Spectrum mean = sum * inv;

  return Rendered{mean, static_cast<double>(re::luminance(mean))};
}

void check_close(const char* what, double a, double b, double tolerance) {
  const double rel = std::fabs(a - b) / std::max(1e-9, std::fabs(a));
  INFO(what << ": naive=" << a << " other=" << b << " rel=" << rel);
  CHECK(rel < tolerance);
}

}  // namespace

TEST_CASE("integrator agreement: nee and mis converge where naive converges") {
  const re::Scene scene = make_bright_box();

  const re::SceneGeometryView geometry = scene.make_geometry_view();

  re::EmbreeAccel accel;
  accel.build(geometry);

  re::SceneView view;
  view.geometry       = geometry;
  view.materials      = scene.material_table();
  view.material_count = static_cast<uint32_t>(scene.materials.size());

  const re::CdfLightData lights = re::build_cdf_lights(view);
  REQUIRE(lights.refs.size() == 2);  // the ceiling, two triangles

  re::TextureCache textures;
  textures.commit(scene.texture_descs);

  const re::CpuServices services(accel, textures);
  const re::CdfLightSelector selector = lights.selector();

  const re::PinholeParams camera = re::make_pinhole(
      {0.0f, 0.0f, 1.4f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 45.0f, kResolution, kResolution);

  const re::RayCone camera_cone = re::make_camera_cone(camera.pixel_spread_angle);

  const Rendered naive = render_mean(camera, [&](const re::Ray& r, re::Sampler& s) {
    return re::li_naive(r, camera_cone, view, services, s, kMaxDepth);
  });
  const Rendered nee = render_mean(camera, [&](const re::Ray& r, re::Sampler& s) {
    return re::li_nee(r, camera_cone, view, selector, services, s, kMaxDepth);
  });
  const Rendered mis = render_mean(camera, [&](const re::Ray& r, re::Sampler& s) {
    return re::li_mis(r, camera_cone, view, selector, services, s, kMaxDepth);
  });

  REQUIRE(naive.luminance > 0.0);

  check_close("nee vs naive", naive.luminance, nee.luminance, 0.02);
  check_close("mis vs naive", naive.luminance, mis.luminance, 0.02);

  // Per channel too, so a colour shift cannot hide inside a matching luminance.
  check_close("nee red",   naive.mean.x, nee.mean.x, 0.03);
  check_close("nee green", naive.mean.y, nee.mean.y, 0.03);
  check_close("nee blue",  naive.mean.z, nee.mean.z, 0.03);
  check_close("mis red",   naive.mean.x, mis.mean.x, 0.03);
  check_close("mis green", naive.mean.y, mis.mean.y, 0.03);
  check_close("mis blue",  naive.mean.z, mis.mean.z, 0.03);
}
