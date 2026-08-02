#pragma once

#include <string>

namespace re {

struct OfflineSettings{
  int width               = 800;
  int height              = 600;
  std::string output_path = "out.png";
  float fov_y_deg         = 45.0f;
};

bool run_offline(const OfflineSettings& settings);

}