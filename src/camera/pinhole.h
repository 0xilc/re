#pragma once

#include "core/platform.h"
#include "core/types.h"
#include "core/math.h"
#include "core/ray.h"

#include <cassert>
#include <cmath>

namespace re
{

struct PinholeParams {
  float3 origin;
  float3 forward;
  float3 right_scaled;
  float3 up_scaled;
  float inv_width;
  float inv_height;
};

inline PinholeParams make_pinhole(const float3& look_from,
                                  const float3& look_at,
                                  const float3& world_up,
                                  float vfov_degrees,
                                  int width, int height) {

  PinholeParams c;
  c.origin = look_from;
  c.forward = normalize(look_at - look_from);
  
  float3 right = cross(c.forward, world_up);
  assert(length_squared(right) > 1e-12f && "Cam forward and the world up cannot be the same.");
  right = normalize(right);
  float3 up = cross(right, c.forward);

  const float tan_half = tanf(vfov_degrees * 0.5f * (kPi / 180.0f));
  const float aspect = float(width) / float(height);

  c.right_scaled = right * (tan_half * aspect);
  c.up_scaled = up * tan_half;
  c.inv_width = 1.0f / float(width);
  c.inv_height = 1.0f / float(height);
  
  return c;
}

RE_HD inline Ray generate_ray(const PinholeParams& c, int px, int py, const float2& jitter) {
  const float sx = (float(px) + jitter.x) * c.inv_width * 2.0f - 1.0f;
  const float sy = 1.0f - (float(py) + jitter.y) * c.inv_height * 2.0f;

  Ray r;
  r.origin = c.origin;
  r.dir = normalize(c.forward + c.right_scaled * sx + c.up_scaled * sy);
  r.tmin = 0.0f;
  r.tmax = kInf;

  return r;
}
}