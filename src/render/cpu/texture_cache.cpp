#include "render/cpu/texture_cache.h"

#include <algorithm>

#include "core/log.h"
#include "scene/texture_desc.h"
#include "texture/texture_build.h"

namespace re {
namespace {
CpuTexture push_placeholder(std::vector<uint8_t>& arena) {
  CpuTexture tex;
  tex.base = static_cast<uint32_t>(arena.size());
  tex.width       = 1;
  tex.height      = 1;
  tex.level_count = 1;
  tex.space       = ColorSpace::Linear;

  arena.insert(arena.end(), {255, 0, 255, 255});
  return tex;
}
} // namespace

void TextureCache::commit(const std::vector<TextureDesc>& descs) {
  arena_.clear();
  textures_.clear();
  textures_.reserve(descs.size());

  for (const TextureDesc& desc : descs) {
    const TextureImage image = build_texture(desc);

    if (!image.ok()) {
      textures_.push_back(push_placeholder(arena_));
      continue;
    }

    CpuTexture tex;
    tex.base        = static_cast<uint32_t>(arena_.size());
    tex.width       = static_cast<uint16_t>(image.width);
    tex.height      = static_cast<uint16_t>(image.height);
    tex.level_count = static_cast<uint8_t>(std::min<size_t>(image.levels.size(), kMaxMipLevels));
    tex.space       = image.space;
    tex.wrap_u      = image.wrap_u;
    tex.wrap_v      = image.wrap_v;

    uint32_t offset = 0;
    for (uint8_t l = 0; l < tex.level_count; ++l) {
      const std::vector<uint8_t>& level = image.levels[l];
      
      tex.mip_offsets[l] = offset;
      arena_.insert(arena_.end(), level.begin(), level.end());
      offset += static_cast<uint32_t>(level.size());
    }

    textures_.push_back(tex);
  }

  log_info("textures: {} committed, {:.1f} MB arena", textures_.size(),
           static_cast<double>(arena_.size()) / (1024.0 * 1024.0));
}

} // namespace re 