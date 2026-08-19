#pragma once 

#include "core/math.h"
#include "core/platform.h"

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

}