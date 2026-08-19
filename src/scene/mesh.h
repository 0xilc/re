#pragma once
#include "core/platform.h"
#include "core/types.h"
#include "core/math.h"
#include <cstdint>

namespace re {

struct MeshView {
  const float3* positions = nullptr;
  const uint32_t* indices = nullptr;
  const uint16_t* mat_ids = nullptr;
  int vertex_count        = 0;
  int triangle_count      = 0;

  RE_HD inline uint16_t get_material_id(int primitive_id) const {
    return mat_ids[primitive_id];
  }
};

struct Triangle {
  float3 v0, v1, v2;

  RE_HD inline float3 geometric_normal() const {
    return normalize(cross(v1 - v0, v2 - v0));
  }

  RE_HD inline float area() const {
    return 0.5f * length(cross(v1 - v0, v2 - v0));
  }

  RE_HD inline float3 get_point(float u, float v) const {
    return v0 + u * (v1 - v0) + v * (v2 - v0);
  }
};

RE_HD inline Triangle get_triangle(const MeshView& view, int primitive_id) {
  uint32_t i0 = view.indices[3 * primitive_id + 0];
  uint32_t i1 = view.indices[3 * primitive_id + 1];
  uint32_t i2 = view.indices[3 * primitive_id + 2];

  return {
    view.positions[i0],
    view.positions[i1],
    view.positions[i2]
  };
}

}