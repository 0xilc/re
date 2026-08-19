#pragma once

#include "core/platform.h"
#include "core/types.h"
#include "core/math.h"
#include "core/transform.h"

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

RE_HD inline Ray transform_ray(const Transform& t, const Ray& ray) {
  Ray r;
  r.origin = transform_point(t, ray.origin);
  r.dir    = transform_vector(t, ray.dir);
  r.tmin   = ray.tmin;
  r.tmax   = ray.tmax;
  return r;
}

}