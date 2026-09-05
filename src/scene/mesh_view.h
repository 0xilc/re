#pragma once

#include <cstdint>

#include "core/math.h"
#include "core/platform.h"
#include "core/types.h"
#include "scene/triangle.h"

namespace re {

struct MeshView {
  const float3* positions = nullptr;
  const float3*   normals = nullptr;
  const float4*  tangents = nullptr;
  const uint32_t* indices = nullptr;
  const uint16_t* mat_ids = nullptr;
  const float2*       uvs = nullptr;
  int vertex_count        = 0;
  int triangle_count      = 0;

  RE_HD inline uint16_t get_material_id(int primitive_id) const {
    return mat_ids[primitive_id];
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

RE_HD inline float2 interpolate_uv(const MeshView& m, int prim_id, float u, float v) {
  if (!m.uvs) return float2{0.0f, 0.0f};

  const float2 uv0 = m.uvs[m.indices[3 * prim_id + 0]];
  const float2 uv1 = m.uvs[m.indices[3 * prim_id + 1]];
  const float2 uv2 = m.uvs[m.indices[3 * prim_id + 2]];

  return barycentric(uv0, uv1, uv2, u, v);
}

RE_HD inline float3 interpolate_normal(const MeshView& m, int prim_id, float u, float v) {
  const uint32_t idx0 = m.indices[3 * prim_id + 0];
  const uint32_t idx1 = m.indices[3 * prim_id + 1];
  const uint32_t idx2 = m.indices[3 * prim_id + 2];

  return normalize(barycentric(m.normals[idx0], m.normals[idx1], m.normals[idx2], u, v));
}

}