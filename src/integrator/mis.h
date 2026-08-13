#pragma once

#include "core/platform.h"


namespace re {

RE_HD inline float power_heuristic(float pdf_a, float pdf_b) {
  const float a = pdf_a * pdf_a;
  const float b = pdf_b * pdf_b;
  const float denom = a + b;

  return denom > 0.0f ? a / denom : 0.0f;
}

}