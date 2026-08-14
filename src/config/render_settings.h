#pragma once

#include <string>
#include "film/tonemap.h"

namespace re {

struct RenderSettings {
  int width     = 800;
  int height    = 600;
  int spp       = 16;
  int max_depth = 8;
  int threads   = 0;

  Tonemap           tonemap = Tonemap::Reinhard;

  std::string scene_path;
  std::string output_path = "out.png";
};

}