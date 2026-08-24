#include "doctest.h"

#include "scene/mesh.h"
#include "scene/mesh_view.h"
#include "scene/scene_builder.h"
#include "scene/scene_view.h"

namespace {

re::Mesh make_uv_quad() {
  re::Mesh mesh;
  mesh.positions = {{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}};
  mesh.uvs       = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
  mesh.indices   = {0, 1, 2, 0, 2, 3};
  mesh.mat_ids   = {0, 0};

  return mesh;
}

}  // namespace

TEST_CASE("interpolate_uv reproduces the vertex uvs at the corners") {
  const re::Mesh     mesh = make_uv_quad();
  const re::MeshView view = re::make_meshview(mesh);

  // Barycentrics weight the edges leaving vertex 0, so (0,0) is v0, (1,0) is v1
  // and (0,1) is v2.
  const re::float2 at_v0 = re::interpolate_uv(view, 0, 0.0f, 0.0f);
  const re::float2 at_v1 = re::interpolate_uv(view, 0, 1.0f, 0.0f);
  const re::float2 at_v2 = re::interpolate_uv(view, 0, 0.0f, 1.0f);

  CHECK(at_v0.x == doctest::Approx(0.0f));
  CHECK(at_v0.y == doctest::Approx(0.0f));
  CHECK(at_v1.x == doctest::Approx(1.0f));
  CHECK(at_v1.y == doctest::Approx(0.0f));
  CHECK(at_v2.x == doctest::Approx(1.0f));
  CHECK(at_v2.y == doctest::Approx(1.0f));
}

TEST_CASE("interpolate_uv is linear inside the triangle") {
  const re::Mesh     mesh = make_uv_quad();
  const re::MeshView view = re::make_meshview(mesh);

  const re::float2 centroid = re::interpolate_uv(view, 0, 1.0f / 3.0f, 1.0f / 3.0f);

  CHECK(centroid.x == doctest::Approx(2.0f / 3.0f));
  CHECK(centroid.y == doctest::Approx(1.0f / 3.0f));
}

TEST_CASE("a mesh without a uv set reads as zero rather than reading garbage") {
  re::Mesh mesh = make_uv_quad();
  mesh.uvs.clear();

  const re::MeshView view = re::make_meshview(mesh);
  REQUIRE(view.uvs == nullptr);

  const re::float2 uv = re::interpolate_uv(view, 0, 0.25f, 0.25f);

  CHECK(uv.x == doctest::Approx(0.0f));
  CHECK(uv.y == doctest::Approx(0.0f));
}

TEST_CASE("uv_at resolves through the instance without transforming the uvs") {
  re::Scene scene;
  scene.add_material(re::make_material({0.5f, 0.5f, 0.5f}, 0.0f, 1.0f));

  const uint32_t mesh_id = scene.add_mesh(make_uv_quad());

  re::Transform shifted = re::transform_identity();
  shifted.r[0].w = 10.0f;
  scene.add_instance(shifted, mesh_id);

  re::SceneView view;
  view.geometry = scene.make_geometry_view();

  const re::float2 uv = view.uv_at(0, 0, 1.0f, 0.0f);

  CHECK(uv.x == doctest::Approx(1.0f));
  CHECK(uv.y == doctest::Approx(0.0f));
}
