#pragma once

#include "core/math.h"
#include "core/platform.h"
#include "core/types.h"

namespace re {

constexpr float kOrigin = 1.0f / 32.0f;
constexpr float kFloatScale = 1.0f / 65536.0f;
constexpr float kIntScale = 256.0f;

namespace {
RE_HD inline int32_t float_as_int(float f) {
#ifdef __CUDA_ARCH__
  return __float_as_int(f);
#else
  int32_t i;
  __builtin_memcpy(&i, &f, sizeof(i));
  return i;
#endif
}

RE_HD inline float int_as_float(int32_t i) {
#ifdef __CUDA_ARCH__
  return __int_as_float(i);
#else
  float f;
  __builtin_memcpy(&f, &i, sizeof(f));
  return f;
#endif
}

}

RE_HD inline float3 offset_ray_origin(const float3& p, const float3& n) {
  const int32_t of_x = static_cast<int32_t>(kIntScale * n.x);
  const int32_t of_y = static_cast<int32_t>(kIntScale * n.y);
  const int32_t of_z = static_cast<int32_t>(kIntScale * n.z);

  const float px = int_as_float(float_as_int(p.x) + (p.x < 0.0f ? -of_x : of_x));
  const float py = int_as_float(float_as_int(p.y) + (p.y < 0.0f ? -of_y : of_y));
  const float pz = int_as_float(float_as_int(p.z) + (p.z < 0.0f ? -of_z : of_z));

  return float3 {
    fabsf(p.x) < kOrigin ? p.x + kFloatScale * n.x : px,
    fabsf(p.y) < kOrigin ? p.y + kFloatScale * n.y : py,
    fabsf(p.z) < kOrigin ? p.z + kFloatScale * n.z : pz,
  };
}

}