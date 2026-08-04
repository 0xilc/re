#pragma once

#include "core/math.h"
#include "core/platform.h"
#include "core/types.h"

namespace re {

RE_HD inline float2 sample_uniform_disk(float2 u) {
  const float a = 2.0f * u.x - 1.0f;
  const float b = 2.0f * u.y - 1.0f;
  if (a == 0.0f && b == 0.0f) return float2 {0.0f, 0.0f};

  float r, phi;
  if (a * a > b * b) {
    r = a;
    phi = (kPi / 4.0f) * (b / a);
  } else {
    r = b;
    phi = (kPi / 2.0f) - (kPi / 4.0f) * (a / b);
  }

  return float2{r * cosf(phi), r * sinf(phi)};
}

RE_HD inline float3 sample_uniform_hemisphere(float2 u) {
  const float z = u.x;
  const float r = safe_sqrt(1.0f - z * z);
  const float phi = kTwoPi * u.y;
  
  return float3{r * cosf(phi), r * sinf(phi), z};
}

RE_HD inline float pdf_uniform_hemisphere(float cos_theta) {
  return cos_theta > 0.0f ? kInvTwoPi : 0.0f;
}

RE_HD inline float3 sample_cosine_hemisphere(float2 u) {
  const float2 d = sample_uniform_disk(u);
  const float z = safe_sqrt(1.0f - d.x * d.x - d.y * d.y);
  
  return float3{d.x, d.y, z};
}

RE_HD inline float pdf_cosine_hemisphere(float cos_theta) {
  return cos_theta > 0.0f ? cos_theta * kInvPi : 0.0f;
}

}