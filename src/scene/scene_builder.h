#pragma once

#include <cstdint>
#include <vector>

#include "scene/mesh.h"
#include "scene/material.h"
#include "light/light_view.h"

namespace re {
struct Scene {
  std::vector<float3> positions;
  std::vector<uint32_t> indices;   // 3 per triangle
  std::vector<uint16_t> mat_ids;   // 1 per triangle
  std::vector<Material> materials;
  std::vector<float> light_cdf;
  std::vector<int> emissive_prims;
  std::vector<int> prim_to_light;

  MeshView geometry() const {
    MeshView g;
    g.positions      = positions.data();
    g.indices        = indices.data();
    g.mat_ids        = mat_ids.data();
    g.vertex_count   = static_cast<int>(positions.size());
    g.triangle_count = static_cast<int>(indices.size() / 3);
    return g;
  }

  LightView light_view() const {
    LightView l;
    l.prims = emissive_prims.data();
    l.cdf = light_cdf.data();
    l.prim_to_light = prim_to_light.data();
    l.count = static_cast<int>(emissive_prims.size());
    
    return l;
  }

  const Material* material_table() const { return materials.data(); }

  uint16_t add_material(const Material& material);

  void add_quad(const float3& p0, const float3& p1, const float3& p2, const float3& p3,
                uint16_t material_id);

  void add_box(const float3& lo, const float3& hi, uint16_t material_id);

  void finalize();
};

Scene make_cornell_scene();

}  // namespace re