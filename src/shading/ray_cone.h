#pragma once

#include <cmath>

#include "core/math.h"
#include "core/platform.h"
#include "core/types.h"

namespace re {

struct RayCone {
  float width   = 0.0f;
  float spread  = 0.0f;
};

RE_HD inline RayCone propagate(RayCone cone, float t) {
  cone.width += cone.spread * t;
  return cone;
}

RE_HD inline float triangle_lod_constant(const float2& uv0, const float2& uv1, const float2& uv2,
                                         float double_world_area) {
  const float2 duv1 = uv1 - uv0;
  const float2 duv2 = uv2 - uv0;

  const float uv_area = fabsf(duv1.x * duv2.y - duv1.y * duv2.x);
  if (!(uv_area > 0.0f) || !(double_world_area > 0.0f)) return 0.0f;

  return 0.5f * log2f(uv_area / double_world_area);
}

RE_HD inline float cone_lod(float lod_constant, const RayCone& cone, const float3& normal,
                            const float3& dir) {
  const float width = fabsf(cone.width);
  if (!(width > 0.0f)) return 0.0f;

  const float cos_theta = fmaxf(fabsf(dot(normal, dir)), 1e-4f);

  return lod_constant + log2f(width / cos_theta);
}

} // namespace re