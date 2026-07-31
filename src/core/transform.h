#pragma once

#include "platform.h"
#include "types.h"
#include "math.h"
#include <cassert>

namespace re {

struct Transform {
  // Alignment: r[i] = { m_i0, m_i1, m_i2, translate_i }
  float4 r[3];
};

RE_HD inline Transform transform_identity() {
  return {
    float4(1.0f, 0.0f, 0.0f, 0.0f),
    float4(0.0f, 1.0f, 0.0f, 0.0f),
    float4(0.0f, 0.0f, 1.0f, 0.0f)
  };
}

RE_HD inline float3 transform_point(const Transform& t, const float3& p) {
  return { dot(t.r[0].xyz, p) + t.r[0].w,
           dot(t.r[1].xyz, p) + t.r[1].w,
           dot(t.r[2].xyz, p) + t.r[2].w };
}

RE_HD inline float3 transform_vector(const Transform& t, const float3& v) {
  return {
    dot(t.r[0].xyz, v),
    dot(t.r[1].xyz, v),
    dot(t.r[2].xyz, v)
  };
}

RE_HD inline float3 transform_normal(const Transform& inv, const float3& n) {
  return normalize(float3{
    inv.r[0].x * n.x + inv.r[1].x * n.y + inv.r[2].x * n.z,
    inv.r[0].y * n.x + inv.r[1].y * n.y + inv.r[2].y * n.z,
    inv.r[0].z * n.x + inv.r[1].z * n.y + inv.r[2].z * n.z
  });
}

RE_HD inline Transform inverse(const Transform& t) {
  float3 r0 = t.r[0].xyz;
  float3 r1 = t.r[1].xyz;
  float3 r2 = t.r[2].xyz;

  float3 c0 = cross(r1, r2);
  float3 c1 = cross(r2, r0);
  float3 c2 = cross(r0, r1);

  float det = dot(r0, c0);

  assert(fabsf(det) > 1e-8f && "Degenerate transform matrix determinant!");
  
  float inv_det = 1.0f / det;

  Transform inv;
  inv.r[0] = float4(c0.x * inv_det, c1.x * inv_det, c2.x * inv_det, 0.0f);
  inv.r[1] = float4(c0.y * inv_det, c1.y * inv_det, c2.y * inv_det, 0.0f);
  inv.r[2] = float4(c0.z * inv_det, c1.z * inv_det, c2.z * inv_det, 0.0f);

  float3 tr = {t.r[0].w, t.r[1].w, t.r[2].w};
  inv.r[0].w = -dot(inv.r[0].xyz, tr);
  inv.r[1].w = -dot(inv.r[1].xyz, tr);
  inv.r[2].w = -dot(inv.r[2].xyz, tr);

  return inv;
}

RE_HD inline Transform compose(const Transform& a, const Transform& b) {
  Transform c;
  for (int i = 0; i < 3; ++i) {
    c.r[i].x = a.r[i].x * b.r[0].x + a.r[i].y * b.r[1].x + a.r[i].z * b.r[2].x;
    c.r[i].y = a.r[i].x * b.r[0].y + a.r[i].y * b.r[1].y + a.r[i].z * b.r[2].y;
    c.r[i].z = a.r[i].x * b.r[0].z + a.r[i].y * b.r[1].z + a.r[i].z * b.r[2].z;
    c.r[i].w = a.r[i].x * b.r[0].w + a.r[i].y * b.r[1].w + a.r[i].z * b.r[2].w + a.r[i].w;
  }
  
  return c;
}

}