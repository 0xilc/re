#pragma once

#include <string>
#include "core/types.h"

namespace re {

struct RenderSettings {
  int width = 800;
  int height = 600;
  int spp = 16;
  int max_depth = 8;

  std::string scene_path;
  std::string output_path = "out.png";
};

}