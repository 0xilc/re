#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "core/types.h"

namespace re {

float linear_to_srgb(float linear);
float srgb_to_linear(float srgb);

std::vector<uint8_t> encode_srgb8(const std::vector<float3>& linear);

bool write_png(const std::string& path, const uint8_t* rgb, int width, int height);

}