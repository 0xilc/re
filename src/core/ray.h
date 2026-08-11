#pragma once

#include "core/platform.h"
#include "core/types.h"
#include "core/math.h"

namespace re {

struct alignas(32) Ray {
  float3 origin;
  float tmin = 0.0f;
  float3 dir;
  float tmax = kInf;
};

RE_HD inline Ray make_ray(const float3& origin, const float3& direction) {
  Ray r;
  r.origin = origin;
  r.dir = direction;
  return r;
}

}