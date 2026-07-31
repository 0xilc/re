#pragma once

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

}