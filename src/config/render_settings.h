#pragma once

#include <string>
#include "film/tonemap.h"
#include "integrator/integrator.h"

namespace re {

struct PathTracerSettings {
  int                   spp = 16;
  int             max_depth = 8;
  IntegratorMode integrator = IntegratorMode::Mis;
};

struct RenderSettings {
  int width     = 800;
  int height    = 600;
  int threads   = 0;

  PathTracerSettings path_tracer;

  Tonemap tonemap = Tonemap::Reinhard;

  std::string scene_path;
  std::string output_path = "out.png";
};

void log_settings(const RenderSettings& settings);
}