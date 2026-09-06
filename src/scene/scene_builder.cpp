#include "scene/scene_builder.h"

#include <utility>

#include "scene/material.h"
#include "scene/scene_view.h"

namespace re {

uint32_t Scene::add_mesh(Mesh mesh) {
  meshes.push_back(std::move(mesh));
  mesh_views.push_back(make_meshview(meshes.back()));

  return static_cast<uint32_t>(meshes.size() - 1);
}

uint32_t Scene::add_instance(Transform to_world, uint32_t mesh_id) {
  assert(mesh_id < meshes.size());

  Instance inst = make_instance(to_world, mesh_id);
  instances.push_back(std::move(inst));
  
  return static_cast<uint32_t>(instances.size() - 1);
}

uint16_t Scene::add_material(const Material& material) {
  materials.push_back(material);
  return static_cast<uint16_t>(materials.size() - 1);
}

void Scene::add_quad(const float3& p0, const float3& p1, const float3& p2, const float3& p3,
                     uint16_t material_id) {
  std::vector<float3> positions;
  positions.push_back(p0);
  positions.push_back(p1);
  positions.push_back(p2);
  positions.push_back(p3);
  std::vector<uint32_t> indices = {
    0, 1, 2, 0, 2, 3
  };
  std::vector<uint16_t> mat_ids = {material_id, material_id};
  Mesh quad;
  quad.positions = positions;
  quad.indices = indices;
  quad.mat_ids = mat_ids;

  uint32_t mesh_id = add_mesh(quad);
  add_instance(transform_identity(), mesh_id);
}

void Scene::add_box(const float3& lo, const float3& hi, uint16_t material_id) {
  add_quad({lo.x, lo.y, hi.z}, {hi.x, lo.y, hi.z}, {hi.x, hi.y, hi.z}, {lo.x, hi.y, hi.z},
           material_id);  // +z
  add_quad({hi.x, lo.y, lo.z}, {lo.x, lo.y, lo.z}, {lo.x, hi.y, lo.z}, {hi.x, hi.y, lo.z},
           material_id);  // -z
  add_quad({hi.x, lo.y, hi.z}, {hi.x, lo.y, lo.z}, {hi.x, hi.y, lo.z}, {hi.x, hi.y, hi.z},
           material_id);  // +x
  add_quad({lo.x, lo.y, lo.z}, {lo.x, lo.y, hi.z}, {lo.x, hi.y, hi.z}, {lo.x, hi.y, lo.z},
           material_id);  // -x
  add_quad({lo.x, hi.y, hi.z}, {hi.x, hi.y, hi.z}, {hi.x, hi.y, lo.z}, {lo.x, hi.y, lo.z},
           material_id);  // +y
  add_quad({lo.x, lo.y, lo.z}, {hi.x, lo.y, lo.z}, {hi.x, lo.y, hi.z}, {lo.x, lo.y, hi.z},
           material_id);  // -y
}

SceneGeometryView Scene::make_geometry_view() const {
  SceneGeometryView view;
  view.meshes         = mesh_views.data();
  view.instances      = instances.data();
  view.mesh_count     = static_cast<uint32_t>(mesh_views.size());
  view.instance_count = static_cast<uint32_t>(instances.size());
  
  return view;
} 

Scene make_cornell_scene() {
  Scene scene;

  constexpr float hx = 0.278f;
  constexpr float hy = 0.274f;
  constexpr float hz = 0.280f;

  const uint16_t white = scene.add_material(make_material({0.73f, 0.73f, 0.73f}, 0.0f, 1.0f));
  const uint16_t red   = scene.add_material(make_material({0.65f, 0.05f, 0.05f}, 0.0f, 1.0f));
  const uint16_t green = scene.add_material(make_material({0.12f, 0.45f, 0.15f}, 0.0f, 1.0f));

  const uint16_t lamp =
      scene.add_material(make_material({0.0f, 0.0f, 0.0f}, 0.0f, 1.0f, {47.0f, 33.0f, 11.0f}));
  scene.add_quad({-hx, -hy, hz}, {hx, -hy, hz}, {hx, -hy, -hz}, {-hx, -hy, -hz}, white);
  scene.add_quad({-hx, hy, -hz}, {hx, hy, -hz}, {hx, hy, hz}, {-hx, hy, hz}, white);
  scene.add_quad({-hx, -hy, -hz}, {hx, -hy, -hz}, {hx, hy, -hz}, {-hx, hy, -hz}, white);
  scene.add_quad({-hx, -hy, -hz}, {-hx, hy, -hz}, {-hx, hy, hz}, {-hx, -hy, hz}, red);
  scene.add_quad({hx, -hy, -hz}, {hx, -hy, hz}, {hx, hy, hz}, {hx, hy, -hz}, green);

  constexpr float lx = 0.065f;
  constexpr float lz = 0.0525f;
  constexpr float ly = hy - 0.001f;
  scene.add_quad({-lx, ly, -lz}, {lx, ly, -lz}, {lx, ly, lz}, {-lx, ly, lz}, lamp);
  scene.add_box({-0.19f, -hy + 1e-4f, -0.17f}, {-0.02f, -hy + 0.33f, 0.00f}, white);
  scene.add_box({0.03f, -hy + 1e-4f, 0.05f}, {0.20f, -hy + 0.165f, 0.22f}, white);

  return scene;
}

}  // namespace re