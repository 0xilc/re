#pragma once 

#include "core/math.h"
#include "core/platform.h"
#include "core/transform.h"

namespace re {

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

// Moves the triangle into another space, e.g. instance local -> world. Taking
// geometric_normal() afterwards is correct even for mirrored transforms, which
// an inverse transpose applied to the local normal would get backwards.
RE_HD inline Triangle transform_triangle(const Transform& t, const Triangle& tri) {
  return {transform_point(t, tri.v0),
          transform_point(t, tri.v1),
          transform_point(t, tri.v2)};
}

}