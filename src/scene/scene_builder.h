#pragma once

#include <cstdint>
#include <vector>

#include "scene/geometry.h"
#include "scene/material.h"

namespace re {
struct Scene {
  std::vector<float3> positions;
  std::vector<uint32_t> indices;   // 3 per triangle
  std::vector<uint16_t> mat_ids;   // 1 per triangle
  std::vector<Material> materials;

  std::vector<int> emissive_prims;

  GeometryView geometry() const {
    GeometryView g;
    g.positions      = positions.data();
    g.indices        = indices.data();
    g.mat_ids        = mat_ids.data();
    g.vertex_count   = static_cast<int>(positions.size());
    g.triangle_count = static_cast<int>(indices.size() / 3);
    return g;
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