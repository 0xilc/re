#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "core/color.h"

namespace re {

enum class WrapMode : uint8_t { Repeat = 0, Clamp = 1 };
enum class TexFilter: uint8_t { Nearest = 0, Bilinear = 1, Trilinear = 2 };

struct TextureDesc {
  std::string          name;   // for logs only
  std::string          path;   // empty when the image is embedded
  std::vector<uint8_t> bytes;  // encoded png/jpeg, when embedded in a .glb

  ColorSpace color_space = ColorSpace::SRGB;
  WrapMode        wrap_u = WrapMode::Repeat;
  WrapMode        wrap_v = WrapMode::Repeat;
  TexFilter       filter = TexFilter::Trilinear;
  
  bool is_normal_map = false;
};

}  // namespace re
