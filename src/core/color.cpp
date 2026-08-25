#include "core/color.h"
#include "io/image_io.h"
#include <array>
#include <cmath>

namespace re {

float srgb_to_linear(float c) {
  return c <= 0.04045f ? c * (1.0f / 12.92f)
                       : std::pow((c + 0.055f) * (1.0f / 1.055f), 2.4f);
}

float linear_to_srgb(float c) {
  return c <= 0.0031308f ? c * 12.92f
                         : 1.055f * std::pow(c, 1.0f / 2.4f) - 0.055f;
}

const float* srgb_lut() {
  static const std::array<float, 256> table = [] {
    std::array<float, 256> a{};
    for (size_t i = 0; i < 256; ++i) a[i] = srgb_to_linear(static_cast<float>(i) / 255.0f);
    return a;
  }();
  return table.data();
}

}