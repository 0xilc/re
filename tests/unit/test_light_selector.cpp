#include "doctest.h"

#include <cmath>
#include <vector>

#include "light/cdf_light_build.h"
#include "scene/scene_builder.h"

namespace {

// Keeps the scene alive next to the views that point into it.
struct LitScene {
  re::Scene             scene;
  re::SceneGeometryView geometry;
  re::SceneView         view;
  re::CdfLightData      lights;

  explicit LitScene(re::Scene s) : scene(std::move(s)) {
    geometry = scene.make_geometry_view();

    view.geometry       = geometry;
    view.materials      = scene.material_table();
    view.material_count = static_cast<uint32_t>(scene.materials.size());

    lights = re::build_cdf_lights(view);
  }

  float pmf_of(int index) const {
    const size_t i = static_cast<size_t>(index);
    return lights.cdf[i + 1] - lights.cdf[i];
  }
};

re::Transform translate_of(float x) {
  re::Transform t = re::transform_identity();
  t.r[0].w = x;
  return t;
}

re::Transform scale_of(float s) {
  re::Transform t = re::transform_identity();
  t.r[0].x = s;
  t.r[1].y = s;
  t.r[2].z = s;
  return t;
}

// One emissive quad, instanced twice, the second one scaled.
re::Scene two_instances_of_one_lamp(float second_scale) {
  re::Scene scene;
  const uint16_t lamp =
      scene.add_material(re::make_material({0.0f, 0.0f, 0.0f}, 0.0f, 1.0f, {10.0f, 10.0f, 10.0f}));

  scene.add_quad({-1, 0, -1}, {1, 0, -1}, {1, 0, 1}, {-1, 0, 1}, lamp);
  scene.add_instance(scale_of(second_scale), 0);

  return scene;
}

}  // namespace

TEST_CASE("light selector: an unlit scene picks nothing") {
  re::Scene scene;
  const uint16_t grey = scene.add_material(re::make_material({0.5f, 0.5f, 0.5f}, 0.0f, 1.0f));
  scene.add_quad({-1, 0, -1}, {1, 0, -1}, {1, 0, 1}, {-1, 0, 1}, grey);

  const LitScene lit{std::move(scene)};
  const re::CdfLightSelector sel = lit.lights.selector();

  CHECK(sel.count == 0);

  const re::LightPick pick = sel.pick(re::LightSampleContext{}, 0.5f);
  CHECK(pick.pmf == 0.0f);
  CHECK(pick.ref.inst_id == re::INVALID_ID);
}

TEST_CASE("light selector: the cdf is a normalised distribution") {
  const LitScene lit{re::make_cornell_scene()};
  const re::CdfLightSelector sel = lit.lights.selector();

  REQUIRE(sel.count > 0);
  REQUIRE(lit.lights.cdf.size() == static_cast<size_t>(sel.count) + 1);

  CHECK(lit.lights.cdf.front() == doctest::Approx(0.0f));
  CHECK(lit.lights.cdf.back() == doctest::Approx(1.0f));

  float sum = 0.0f;
  for (int i = 0; i < sel.count; ++i) {
    const float pmf = lit.pmf_of(i);
    CHECK(pmf >= 0.0f);  // monotone, so no negative bins
    sum += pmf;
  }

  CHECK(sum == doctest::Approx(1.0f));
}

// The one that keeps MIS honest: what pick() reports and what pmf() reports for
// the same light have to be the same number, or the weights are quietly wrong.
TEST_CASE("light selector: pick and pmf agree on every light") {
  const LitScene lit{re::make_cornell_scene()};
  const re::CdfLightSelector sel = lit.lights.selector();
  REQUIRE(sel.count > 0);

  const re::LightSampleContext ctx{};

  int disagreements = 0;
  int unreachable   = 0;
  std::vector<int> hits(static_cast<size_t>(sel.count), 0);

  constexpr int kSamples = 10000;
  for (int k = 0; k < kSamples; ++k) {
    const float u = (static_cast<float>(k) + 0.5f) / static_cast<float>(kSamples);
    const re::LightPick pick = sel.pick(ctx, u);

    REQUIRE(pick.pmf > 0.0f);
    if (pick.pmf != sel.pmf(ctx, pick.ref)) ++disagreements;

    bool found = false;
    for (int i = 0; i < sel.count; ++i) {
      if (lit.lights.refs[static_cast<size_t>(i)].inst_id == pick.ref.inst_id &&
          lit.lights.refs[static_cast<size_t>(i)].prim_id == pick.ref.prim_id) {
        ++hits[static_cast<size_t>(i)];
        found = true;
        break;
      }
    }
    if (!found) ++unreachable;
  }

  CHECK(disagreements == 0);
  CHECK(unreachable == 0);

  // Stratified u over [0,1) must visit every bin in proportion to its pmf.
  for (int i = 0; i < sel.count; ++i) {
    const float measured = static_cast<float>(hits[static_cast<size_t>(i)]) / kSamples;
    CHECK(measured == doctest::Approx(lit.pmf_of(i)).epsilon(0.01));
  }
}

TEST_CASE("light selector: a hit that is not a light has zero pmf") {
  const LitScene lit{re::make_cornell_scene()};
  const re::CdfLightSelector sel = lit.lights.selector();
  const re::LightSampleContext ctx{};

  CHECK(sel.pmf(ctx, re::LightRef{0, 0}) == 0.0f);            // cornell floor
  CHECK(sel.pmf(ctx, re::LightRef{9999, 0}) == 0.0f);         // no such instance
  CHECK(sel.pmf(ctx, re::LightRef{re::INVALID_ID, re::INVALID_ID}) == 0.0f);
}

// With two or four lights a broken search still stumbles onto the right answer.
// This one is deep enough that it cannot.
TEST_CASE("light selector: reverse lookup finds every light and nothing else") {
  constexpr int kExtraInstances = 64;

  re::Scene scene;
  const uint16_t lamp = scene.add_material(
      re::make_material({0.0f, 0.0f, 0.0f}, 0.0f, 1.0f, {10.0f, 10.0f, 10.0f}));
  const uint16_t grey = scene.add_material(re::make_material({0.5f, 0.5f, 0.5f}, 0.0f, 1.0f));

  scene.add_quad({-1, 0, -1}, {1, 0, -1}, {1, 0, 1}, {-1, 0, 1}, lamp);  // mesh 0, instance 0
  scene.add_quad({-1, 2, -1}, {1, 2, -1}, {1, 2, 1}, {-1, 2, 1}, grey);  // mesh 1, instance 1
  for (int i = 0; i < kExtraInstances; ++i) {
    scene.add_instance(translate_of(3.0f * static_cast<float>(i + 1)), 0);
  }

  const LitScene lit{std::move(scene)};
  const re::CdfLightSelector sel = lit.lights.selector();
  const re::LightSampleContext ctx{};

  REQUIRE(sel.count == 2 * (1 + kExtraInstances));

  for (int i = 0; i < sel.count; ++i) {
    const re::LightRef ref = lit.lights.refs[static_cast<size_t>(i)];
    CHECK(sel.pmf(ctx, ref) == lit.pmf_of(i));
  }

  CHECK(sel.pmf(ctx, re::LightRef{1, 0}) == 0.0f);  // the grey quad's instance
  CHECK(sel.pmf(ctx, re::LightRef{1, 1}) == 0.0f);
  CHECK(sel.pmf(ctx, re::LightRef{0, 7}) == 0.0f);  // past the end of a lit mesh
  CHECK(sel.pmf(ctx, re::LightRef{999, 0}) == 0.0f);
}

TEST_CASE("light selector: identical instances of one lamp split the weight evenly") {
  const LitScene lit{two_instances_of_one_lamp(1.0f)};
  const re::CdfLightSelector sel = lit.lights.selector();

  REQUIRE(sel.count == 4);  // one quad is two triangles, instanced twice
  for (int i = 0; i < sel.count; ++i) CHECK(lit.pmf_of(i) == doctest::Approx(0.25f));
}

// The instance transform scales the emitting area, so it has to scale how often
// the light gets picked. Using the untransformed triangle would split evenly.
TEST_CASE("light selector: a scaled instance is weighted by its world area") {
  constexpr float kScale = 2.0f;
  const LitScene lit{two_instances_of_one_lamp(kScale)};
  const re::CdfLightSelector sel = lit.lights.selector();

  REQUIRE(sel.count == 4);

  const float unscaled = lit.pmf_of(0) + lit.pmf_of(1);
  const float scaled   = lit.pmf_of(2) + lit.pmf_of(3);

  // Area grows with the square of a uniform scale.
  CHECK(scaled / unscaled == doctest::Approx(kScale * kScale));
}
