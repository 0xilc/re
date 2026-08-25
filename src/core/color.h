#pragma once

#include <cstdint>

namespace re {

enum class ColorSpace : uint8_t {SRGB = 0, Linear = 1};

float srgb_to_linear(float c);
float linear_to_srgb(float c);

const float* srgb_lut();
}