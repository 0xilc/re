#pragma once

#include "platform.h"
#include "types.h"
#include "math.h"

namespace re {

struct Frame {
  float3 s; // Tangent
  float3 t; // Bitangent
  float3 n; // Normal

  RE_HD inline float3 to_world(const float3& v) const {
    return s * v.x + t * v.y + n * v.z;
  }

  RE_HD inline float3 to_local(const float3& v) const {
    return { dot(v, s), dot(v, t), dot(v, n) };
  }
};

// Building an Orthonormal Basis, Revisited, Duff et al.
// [https://jcgt.org/published/0006/01/01/paper-lowres.pdf]
RE_HD inline Frame make_frame(const float3& n) {
  Frame f;
  f.n = n;

  float sign = copysignf(1.0f, n.z);
  const float a = -1.0f / (sign + n.z);
  const float b = n.x * n.y * a;

  f.s = { 1.0f + sign * n.x * n.x * a, sign * b, -sign * n.x };
  f.t = { b, sign + n.y * n.y * a, -n.y };

  return f;
}

// Operate on vectors that are in local space 
namespace local {
  RE_HD inline float cos_theta(const float3& w)     { return w.z; }
  RE_HD inline float abs_cos_theta(const float3& w) { return fabsf(w.z); }
  RE_HD inline float cos2_theta(const float3& w)    { return w.z * w.z; }
  RE_HD inline float sin2_theta(const float3& w)    { return fmaxf(0.0f, 1.0f - w.z * w.z); }
  RE_HD inline float tan2_theta(const float3& w)    { return sin2_theta(w) / (w.z * w.z); }
  RE_HD inline bool  same_hemisphere(const float3& a, const float3& b) { return a.z * b.z > 0.0f; }
};

}