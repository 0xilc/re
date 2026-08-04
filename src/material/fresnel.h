#pragma once

#include "core/spectrum.h"
#include "core/math.h"

namespace re {

RE_HD inline Spectrum fresnel_schlick(const Spectrum& f0, float cos_theta_h) {
  const float m = fmaxf(0.0f, 1.0f - cos_theta_h);
  const float m2 = m * m;
  const float m5 = m2 * m2 * m;

  return f0 + (Spectrum{1.0f, 1.0f, 1.0f} - f0) * m5;
}

}