#include "scene/scene_builder.h"
#include "scene/material.h"
#include "light/light_picker_build.h"

namespace re {

uint16_t Scene::add_material(const Material& material) {
  materials.push_back(material);
  return static_cast<uint16_t>(materials.size() - 1);
}

void Scene::add_quad(const float3& p0, const float3& p1, const float3& p2, const float3& p3,
                     uint16_t material_id) {
  const uint32_t base = static_cast<uint32_t>(positions.size());
  positions.push_back(p0);
  positions.push_back(p1);
  positions.push_back(p2);
  positions.push_back(p3);

  const uint32_t tri[] = {base + 0, base + 1, base + 2, base + 0, base + 2, base + 3};
  for (uint32_t i : tri) indices.push_back(i);
  mat_ids.push_back(material_id);
  mat_ids.push_back(material_id);
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

void Scene::finalize() {
  emissive_prims.clear();

  const int triangle_count = static_cast<int>(indices.size() / 3);
  for (int prim = 0; prim < triangle_count; ++prim) {
    if (materials[mat_ids[static_cast<size_t>(prim)]].is_emissive()) {
      emissive_prims.push_back(prim);
    }
  }

  light_cdf = build_light_cdf(geometry(), material_table(), emissive_prims);
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
  scene.add_box({-0.19f, -hy, -0.17f}, {-0.02f, -hy + 0.33f, 0.00f}, white);
  scene.add_box({0.03f, -hy, 0.05f}, {0.20f, -hy + 0.165f, 0.22f}, white);

  scene.finalize();
  return scene;
}

}  // namespace re