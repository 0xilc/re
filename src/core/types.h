#pragma once

#include "platform.h"

namespace re {

struct alignas(8) float2 {
  float x, y;

  RE_HD float2() = default;
  RE_HD float2(float v): x(v), y(v) {}
  RE_HD float2(float x_, float y_): x(x_), y(y_) {}
};

struct alignas(4) float3 {
  float x, y, z;
  
  RE_HD float3() = default;
  RE_HD explicit float3(float v): x(v), y(v), z(v) {}
  RE_HD float3(float x_, float y_, float z_): x(x_), y(y_), z(z_) {}
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
struct alignas(16) float4 {
  union {
    struct { float x, y, z, w; };
    float3 xyz;
  };
  
  RE_HD float4() = default;
  RE_HD float4(float v): x(v), y(v), z(v), w(v) {}
  RE_HD float4(float x_, float y_, float z_, float w_): x(x_), y(y_), z(z_), w(w_) {}
  RE_HD float4(const float3& xyz_, float w_): x(xyz_.x), y(xyz_.y), z(xyz_.z), w(w_) {}
};
#pragma GCC diagnostic pop
}