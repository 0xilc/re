#pragma once

#include "core/math.h"
#include "core/platform.h"
#include "core/spectrum.h"
#include "core/types.h"

namespace re {

// --- Conventions
// a: local frame
// b: wi, wo point away from the surface
// c: eval() returns bsdf value alone, exclutdes the cosine
// d: pdf is with respect to solid angle
// e invalid sample == pdf 0, caller must not read wi or f.
struct BsdfSample{
  float3 wi;
  Spectrum f;
  float pdf;
};

RE_HD inline BsdfSample invalid_sample() {
  return BsdfSample {float3{0.0f, 0.0f, 1.0f}, Spectrum{0.0f, 0.0f, 0.0f}, 0.0f};
}

RE_HD inline float3 reflect(const float3& w, const float3& n) {
  return 2.0f * dot(w, n) * n - w;
}

}