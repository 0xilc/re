#pragma once

#include "types.h"
#include <cmath>

namespace re {

// Scalar Math
RE_HD inline float clamp(float v, float lo, float hi) {
  return fmaxf(lo, fminf(v, hi));
}

RE_HD inline float safe_sqrt(float v) {
  return sqrtf(fmaxf(v, 0.0f));
}

RE_HD inline float lerp(float a, float b, float t) {
  return a + t * (b - a);
}

// Vector Overloads
RE_HD inline float3 operator+(float3 a, float3 b) {
    return float3(a.x + b.x, a.y + b.y, a.z + b.z);
}

RE_HD inline float3 operator-(float3 a, float3 b) {
    return float3(a.x - b.x, a.y - b.y, a.z - b.z);
}

RE_HD inline float3 operator*(float3 a, float3 b) {
    return float3(a.x * b.x, a.y * b.y, a.z * b.z);
}

RE_HD inline float3 operator*(float3 a, float s) {
    return float3(a.x * s, a.y * s, a.z * s);
}

RE_HD inline float3 operator/(float3 a, float s) {
    float inv = 1.0f / s;
    return float3(a.x * inv, a.y * inv, a.z * inv);
}

RE_HD inline float4 operator+(float4 a, float4 b) {
    return float4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

RE_HD inline float4 operator-(float4 a, float4 b) {
    return float4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

RE_HD inline float4 operator*(float4 a, float4 b) {
    return float4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}

RE_HD inline float4 operator*(float4 a, float s) {
    return float4(a.x * s, a.y * s, a.z * s, a.w * s);
}

RE_HD inline float4 operator/(float4 a, float s) {
    float inv = 1.0f / s;
    return float4(a.x * inv, a.y * inv, a.z * inv, a.w * inv);
}

// Vector Math
RE_HD inline float dot(float3 a, float3 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

RE_HD inline float3 cross(float3 a, float3 b) {
  return float3(
    a.y * b.z - a.z * b.y,
    a.z * b.x - a.x * b.z,
    a.x * b.y - a.y * b.x
  );
}

RE_HD inline float length_squared(float3 v) {
  return dot(v, v);
}

RE_HD inline float length(float3 v) {
  return safe_sqrt(length_squared(v));
}

RE_HD inline float3 normalize(float3 v) {
  float len_sq = length_squared(v);
  if (len_sq == 0.0f) {
    return float3(0.0f);
  }
  float inv_len = 1.0f / sqrtf(len_sq);
  return v * inv_len;
}

RE_HD inline float3 lerp(float3 a, float3 b, float t) {
  return a + (b - a) * t;
}
}