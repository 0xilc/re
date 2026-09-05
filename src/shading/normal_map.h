#pragma once

#include "core/platform.h"
#include "core/types.h"
#include "core/frame.h"

namespace re {
RE_HD inline float3 apply_normal_map(
    const float4& texel,
    float scale,
    const Frame& frame)
{
  float3 n{
      texel.x * 2.0f - 1.0f,
      texel.y * 2.0f - 1.0f,
      texel.z * 2.0f - 1.0f
  };

  n.x *= scale;
  n.y *= scale;

  if (length_squared(n) < 1e-12f)
    return frame.n;

  return normalize(frame.to_world(normalize(n)));
};

} // namespace re