#pragma once

#include "core/platform.h"
#include "core/types.h"

namespace re {

namespace {
  inline constexpr float kLumaR = 0.2126f;
  inline constexpr float kLumaG = 0.7152f;
  inline constexpr float kLumaB = 0.0722f;
}

using Spectrum = float3;

RE_HD inline Spectrum make_empty_spectrum() {
  return Spectrum{0.0f, 0.0f, 0.0f};
}

RE_HD inline float luminance(const Spectrum& s) {
  return kLumaR * s.x + kLumaG * s.y + kLumaB * s.z;
}

}