#pragma once
#include <vector>
#include <cstdint>

#include "core/color.h"
#include "scene/texture_desc.h"
#include "io/image_io.h"

namespace re {

struct TextureImage {
  std::vector<std::vector<uint8_t>> levels;
  uint32_t width      = 0;
  uint32_t height     = 0;
  ColorSpace space    = ColorSpace::SRGB;
  WrapMode wrap_u     = WrapMode::Repeat;
  WrapMode wrap_v     = WrapMode::Repeat;
  TexFilter filter    = TexFilter::Trilinear;
  ImageFormat format  = ImageFormat::RGBA8;

  bool ok() const { return !levels.empty(); }
};

TextureImage build_texture(const TextureDesc& desc);

inline uint32_t mip_size(uint32_t base, uint32_t level) {
  if (level >= 32) return 1;
  const uint32_t size = base >> level;
  return size > 0 ? size : 1;
}

}
