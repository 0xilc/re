#pragma once

#include "platform.h"

namespace re {

struct alignas(8) float2 {
  float x, y;

  RE_HD float2(): x(0.0f), y(0.0f) {}
  RE_HD float2(float v): x(v), y(v) {}
  RE_HD float2(float x_, float y_): x(x_), y(y_) {}
};

struct alignas(4) float3 {
  float x, y, z;
  
  RE_HD float3(): x(0.0f), y(0.0f), z(0.0f) {}
  RE_HD float3(float v): x(v), y(v), z(v) {}
  RE_HD float3(float x_, float y_, float z_): x(x_), y(y_), z(z_) {}
};

struct alignas(16) float4 {
  float x, y, z, w;
  
  RE_HD float4(): x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
  RE_HD float4(float v): x(v), y(v), z(v), w(v) {}
  RE_HD float4(float x_, float y_, float z_, float w_): x(x_), y(y_), z(z_), w(w_) {}
};
}