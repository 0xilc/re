#pragma once

#include "core/platform.h"
#include "core/spectrum.h"
#include "core/math.h"

namespace re {

namespace {
  constexpr int   kRRMinDepth       = 3;
  constexpr float kRRMaxProbability = 0.95f;
} // namespace

RE_HD inline bool russian_roulette(Spectrum& throughput, int depth, float u) {
  if (depth < kRRMinDepth) return true;

  const float m = fmaxf(throughput.x, fmaxf(throughput.y, throughput.z));
  const float p = fminf(m, kRRMaxProbability);

  if (!(p > 0.0f)) return false;
  if (u >= p) return false;

  throughput = throughput * (1.0f / p);
  return true;
}
}