#pragma once

#include <cstdint>

#include "core/platform.h"

namespace re {

inline constexpr uint64_t kInvalidTexture = ~static_cast<uint64_t>(0);
inline constexpr uint16_t kNoTexture = 0xFFFF;

enum class ColorSpace : uint8_t { Srgb = 0, Linear = 1 };
enum class WrapMode : uint8_t { Repeat = 0, Clamp = 1 };
enum class TexelFormat : uint8_t { Srgb8 = 0, Linear8 = 1, Float32 = 2 };
enum class TexelStorage : uint8_t { Bytes = 0, Float = 1 };

struct alignas(8) TextureView {
  uint64_t handle    = kInvalidTexture;
  uint16_t width     = 0;
  uint16_t height    = 0;
  uint8_t  mip_count = 0;
  uint8_t  channels  = 0;
  WrapMode wrap      = WrapMode::Repeat;
  uint8_t  reserved  = 0;
};

RE_HD inline bool is_valid_texture(const TextureView& tex) {
  return tex.handle != kInvalidTexture && tex.mip_count > 0;
}

}  // namespace re
