#pragma once

#include "platform.h"
#include "types.h"
#include "math.h"

namespace re {

struct AABB {
  float3 hi;
  float3 lo;
};

RE_HD inline AABB aabb_empty() {
  return {
    { +INF, +INF, +INF },
    { -INF, -INF, -INF }
  };
}

RE_HD inline void expand(AABB& b, const float3& p) {
  b.lo = min3(b.lo, p);
  b.hi = max3(b.hi, p);
}

RE_HD inline AABB merge(const AABB& a, const AABB& b) {
  return { min3(a.lo, b.lo), max3(a.hi, b.hi) };
}

RE_HD inline float3 center(const AABB& b) { return (b.lo + b.hi) * 0.5f; }

RE_HD inline float3 extent(const AABB& b) { return b.hi - b.lo; }

RE_HD inline float surface_area(const AABB& b) {
  float3 e = extent(b);
  return 2.0f * (e.x * e.y + e.y * e.z + e.z * e.x);
}

RE_HD inline bool hit_aabb(const AABB& b, 
                          const float3& origin,
                          const float3& inv_dir,
                          float tmin, float tmax) {
  float tx0 = (b.lo.x - origin.x) * inv_dir.x;
  float tx1 = (b.hi.x - origin.x) * inv_dir.x;
  tmin = fmaxf(tmin, fminf(tx0, tx1));
  tmax = fminf(tmax, fmaxf(tx0, tx1));

  float ty0 = (b.lo.y - origin.y) * inv_dir.y;
  float ty1 = (b.hi.y - origin.y) * inv_dir.y;
  tmin = fmaxf(tmin, fminf(ty0, ty1));
  tmax = fminf(tmax, fmaxf(ty0, ty1));

  float tz0 = (b.lo.z - origin.z) * inv_dir.z;
  float tz1 = (b.hi.z - origin.z) * inv_dir.z;
  tmin = fmaxf(tmin, fminf(tz0, tz1));
  tmax = fminf(tmax, fmaxf(tz0, tz1));

  return tmin <= tmax;
}

}