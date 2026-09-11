#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "core/types.h"

namespace re {
inline constexpr uint32_t kNumChannels = 4;
enum class ImageFormat : uint8_t { RGBA8 = 0, RGBA32F = 1 };

struct Image {
  std::vector<uint8_t>  data;
  uint32_t              width = 0; 
  uint32_t              height = 0;
  ImageFormat           format;

  bool ok() const { return width && height && !data.empty(); }
};

Image read_image(const std::string& path);
Image read_image(const uint8_t* bytes, size_t n);

std::vector<uint8_t> encode_srgb8(const std::vector<float3>& linear);
bool write_png(const std::string& path, const uint8_t* rgb, int width, int height);

inline size_t texel_size(ImageFormat format) {
  switch (format) {
    case ImageFormat::RGBA8:   return kNumChannels * sizeof(uint8_t);
    case ImageFormat::RGBA32F: return kNumChannels * sizeof(float);
  }

  return 0;
}

}
