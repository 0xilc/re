#pragma once

#include <cstdint>
#include <vector>

#include "core/color.h"
#include "scene/texture_desc.h"

inline constexpr uint32_t kMaxMipLevels = 16;

namespace re {
struct CpuTexture {
  uint32_t  base = 0;
  uint32_t  mip_offsets[kMaxMipLevels] = {};
  uint16_t  width = 0, height = 0;
  uint8_t   level_count = 0;
  ColorSpace space = ColorSpace::SRGB;
  WrapMode wrap_u = WrapMode::Repeat;
  WrapMode wrap_v = WrapMode::Repeat;
  TexFilter filter = TexFilter::Trilinear;

  bool ok() const { return level_count > 0; }
};

class TextureCache {
public:
  void commit(const std::vector<TextureDesc>& descs);
  
  const uint8_t*    texels() const { return arena_.data(); }
  const CpuTexture* textures() const { return textures_.data(); }
  uint32_t          texture_count() const { return static_cast<uint32_t>(textures_.size()); }
  size_t            arena_bytes() const { return arena_.size(); }

private:
  std::vector<uint8_t> arena_;
  std::vector<CpuTexture> textures_;
};

} // namespace re