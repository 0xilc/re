#pragma once

#include "core/math.h"
#include "core/sampling.h"
#include "core/frame.h"

namespace re {

inline constexpr float kMinAlpha = 1e-3f;

RE_HD inline float ggx_d(const float3& wm, float alpha) {
  const float c2 = local::cos2_theta(wm);
  const float a2 = alpha * alpha;
  const float t = (a2 - 1.0f) * c2 + 1.0f;
  return a2 / (kPi * t * t);
}

// Smith's lambda
RE_HD inline float ggx_lambda(const float3& w, float alpha) {
  const float c2 = local::cos2_theta(w);
  if (c2 == 0.0f) return INFINITY;
  const float t2 = local::sin2_theta(w) / c2;
  return 0.5f * (safe_sqrt(1.0f + alpha * alpha * t2) - 1.0f);
}

RE_HD inline float ggx_g1(const float3& w, float alpha) {
  return 1.0f / (1.0f + ggx_lambda(w, alpha));
}

// Height correlated
RE_HD inline float ggx_g2(const float3& wo, const float3& wi, float alpha) {
  return 1.0f / (1.0f + ggx_lambda(wo, alpha) + ggx_lambda(wi, alpha));
}

// Requires wo.z > 0; the caller flips into the upper hemisphere and
// flips the result back.
RE_HD inline float3 ggx_sample_vndf(const float3& wo, float alpha, float2 u) {
  const float3 vh = normalize(float3{alpha * wo.x, alpha * wo.y, wo.z});

  const float lensq = vh.x * vh.x + vh.y * vh.y;
  const float3 t1    = lensq > 0.0f ? float3{-vh.y, vh.x, 0.0f} * (1.0f / safe_sqrt(lensq))
                                    : float3{1.0f, 0.0f, 0.0f};

  const float3 t2 = cross(vh, t1);

  const float2 d = sample_uniform_disk(u);
  const float s = 0.5f * (1.0f + vh.z);
  const float y = (1.0f - s) * safe_sqrt(1.0f - d.x * d.x) + s * d.y;

  const float3 nh = d.x * t1 + y * t2 + safe_sqrt(1.0f - d.x * d.x - y * y) * vh;

  return normalize(float3{alpha * nh.x, alpha * nh.y, fmaxf(0.0f, nh.z)});
}

RE_HD inline float ggx_vndf_pdf(const float3& wo, const float3& wm, float alpha) {
  const float cos_o = local::abs_cos_theta(wo);
  const float dot_wo_wm = dot(wo, wm);
  if (cos_o == 0.0f || dot_wo_wm <= 0.0f) return 0.0f;
  return ggx_g1(wo, alpha) * ggx_d(wm, alpha) * dot_wo_wm / cos_o;
}
}