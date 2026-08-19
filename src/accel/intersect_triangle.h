#pragma once

#include "core/platform.h"
#include "core/types.h"
#include "core/math.h"
#include "core/ray.h"
#include "scene/triangle.h"

namespace re {

constexpr float TRIANGLE_INTERSECTION_EPS = 1e-8f;

struct TriangleHit {
  float t;
  float u, v;
};

// Ray dir should be normalized
// No backface culling
// [Möller-Trumbore Intersection Algorithm]
RE_HD inline bool intersect_triangle(const Ray& ray, const Triangle& tri, TriangleHit& out) {
  const float3 e1 = tri.v1 - tri.v0;
  const float3 e2 = tri.v2 - tri.v0;

  const float3 pvec = cross(ray.dir, e2);
  const float det = dot(e1, pvec);

  if (fabsf(det) < TRIANGLE_INTERSECTION_EPS) return false;

  const float3 tvec = ray.origin - tri.v0;
  
  const float sgn = (det < 0.0f) ? -1.0f : 1.0f;
  const float abs_det = det * sgn;
  
  const float su = dot(tvec, pvec) * sgn;
  if (su < 0.0f || su > abs_det) return false;
  
  const float3 qvec = cross(tvec, e1);
  const float sv = dot(ray.dir, qvec) * sgn;
  if (sv < 0.0f || su + sv > abs_det) return false;

  const float st = dot(e2, qvec) * sgn;
  if (st <= ray.tmin * abs_det || st >= ray.tmax * abs_det) return false;

  const float inv_det = 1.0f / abs_det;
  out.t = st * inv_det;
  out.u = su * inv_det;
  out.v = sv * inv_det;

  return true;
}

}
