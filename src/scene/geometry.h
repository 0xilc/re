#pragma once
#include "core/platform.h"
#include "core/types.h"
#include <cstdint>

namespace re {

struct GeometryView {
  const float3* positions = nullptr;
  const uint32_t* indices = nullptr;
  int vertex_count        = 0;
  int triangle_count      = 0;
};

struct Triangle {
  float3 v0, v1, v2;
};

RE_HD inline Triangle get_triangle(const GeometryView& view, int primitive_id) {
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