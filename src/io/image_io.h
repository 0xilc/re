#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "core/types.h"

namespace re {

struct ImageData {
  std::vector<uint8_t> pixels;
  uint32_t width = 0, height = 0;
  bool ok() const { return width && height && !pixels.empty(); }
};

ImageData read_image(const std::string& path);
ImageData read_image(const uint8_t* bytes, size_t n);

std::vector<uint8_t> encode_srgb8(const std::vector<float3>& linear);
bool write_png(const std::string& path, const uint8_t* rgb, int width, int height);

}