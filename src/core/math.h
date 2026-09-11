#pragma once

#include "platform.h"
#include "types.h"
#include <cmath>

namespace re {

// Numerical Constants
inline constexpr float kInf         = INFINITY;
inline constexpr float kPi          = 3.14159265358979323846f;
inline constexpr float kTwoPi       = 6.28318530717958647692f;
inline constexpr float kInvPi       = 0.31830988618379067154f;
inline constexpr float kInvTwoPi    = 0.15915494309189533577f;

// Scalar Math
RE_HD inline float clamp(const float v, const float lo, const float hi) {
  return fmaxf(lo, fminf(v, hi));
}

RE_HD inline float safe_sqrt(const float v) {
  return sqrtf(fmaxf(v, 0.0f));
}

RE_HD inline float lerp(const float a, const float b, const float t) {
  return a + t * (b - a);
}

// Vector Overloads
RE_HD inline float2 operator+(const float2 a, const float2 b) {
    return float2(a.x + b.x, a.y + b.y);
}

RE_HD inline float2 operator-(const float2 a, const float2 b) {
    return float2(a.x - b.x, a.y - b.y);
}

RE_HD inline float2 operator*(const float2 a, const float2 b) {
    return float2(a.x * b.x, a.y * b.y);
}

RE_HD inline float2 operator*(const float2 a, const float s) {
    return float2(a.x * s, a.y * s);
}

RE_HD inline float2 operator*(const float s, const float2 a) {
    return float2(a.x * s, a.y * s);
}

RE_HD inline float3 operator+(const float3 a, const float3 b) {
    return float3(a.x + b.x, a.y + b.y, a.z + b.z);
}

RE_HD inline float3 operator+(const float3 a, const float s) {
    return float3(a.x + s, a.y + s, a.z + s);
}

RE_HD inline float3 operator-(const float3 a, const float3 b) {
    return float3(a.x - b.x, a.y - b.y, a.z - b.z);
}

RE_HD inline float3 operator*(const float3 a, const float3 b) {
    return float3(a.x * b.x, a.y * b.y, a.z * b.z);
}

RE_HD inline float3 operator*(const float3 a, const float s) {
    return float3(a.x * s, a.y * s, a.z * s);
}

RE_HD inline float3 operator/(const float3 a, const float s) {
    const float inv = 1.0f / s;
    return float3(a.x * inv, a.y * inv, a.z * inv);
}

RE_HD inline float3 operator*(const float s, const float3 a) {
    return float3(a.x * s, a.y * s, a.z * s);
}

RE_HD inline float3 operator/(const float s, const float3 a) {
    return float3(s / a.x, s / a.y, s / a.z);
}

RE_HD inline float3 operator-(const float3 a) {
    return float3(-a.x, -a.y, -a.z);
}

RE_HD inline float4 operator+(const float4 a, const float4 b) {
    return float4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

RE_HD inline float4 operator-(const float4 a, const float4 b) {
    return float4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

RE_HD inline float4 operator*(const float4 a, const float4 b) {
    return float4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}

RE_HD inline float4 operator*(const float4 a, const float s) {
    return float4(a.x * s, a.y * s, a.z * s, a.w * s);
}

RE_HD inline float4 operator/(const float4 a, const float s) {
    const float inv = 1.0f / s;
    return float4(a.x * inv, a.y * inv, a.z * inv, a.w * inv);
}

RE_HD inline float4 operator*(const float s, const float4 a) {
    return float4(a.x * s, a.y * s, a.z * s, a.w * s);
}

RE_HD inline float4 operator/(const float s, const float4 a) {
    return float4(s / a.x, s / a.y, s / a.z, s / a.w);
}

// Vector Math
RE_HD inline float dot(const float3 a, const float3 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

RE_HD inline float3 cross(const float3 a, const float3 b) {
  return float3(
    a.y * b.z - a.z * b.y,
    a.z * b.x - a.x * b.z,
    a.x * b.y - a.y * b.x
  );
}

RE_HD inline float length_squared(const float3 v) {
  return dot(v, v);
}

RE_HD inline float length(const float3 v) {
  return safe_sqrt(length_squared(v));
}

RE_HD inline float3 normalize(const float3 v) {
  const float len_sq = length_squared(v);
  if (len_sq == 0.0f) {
    return float3(0.0f);
  }
  const float inv_len = 1.0f / sqrtf(len_sq);
  return v * inv_len;
}

RE_HD inline float3 lerp(const float3 a, const float3 b, const float t) {
  return a + (b - a) * t;
}

RE_HD inline float4 lerp(const float4 a, const float4 b, const float t) {
  return a + (b - a) * t;
}

RE_HD inline float3 min3(const float3 a, const float3 b) {
  return float3(
    fminf(a.x, b.x),
    fminf(a.y, b.y),
    fminf(a.z, b.z)
  );
}

RE_HD inline float3 max3(const float3 a, const float3 b) {
  return float3(
    fmaxf(a.x, b.x),
    fmaxf(a.y, b.y),
    fmaxf(a.z, b.z)
  );
}
}