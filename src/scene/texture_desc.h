#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "shading/texture_view.h"

namespace re {

struct TextureDesc {
  std::string          name;   // for logs only
  std::string          path;   // empty when the image is embedded
  std::vector<uint8_t> bytes;  // encoded png/jpeg, when embedded in a .glb

  ColorSpace color_space = ColorSpace::Srgb;
  WrapMode   wrap        = WrapMode::Repeat;
};

}  // namespace re
