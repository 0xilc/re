#pragma once

#include "camera/pinhole.h"
#include "film/film.h"
#include "render/cpu/cpu_services.h"
#include "scene/scene_view.h"

namespace re {

struct PathRenderSettings {
  int samples_per_pixel = 64;
  int max_depth = 8;
};

class CpuPathRenderer {
public:
  CpuPathRenderer(const SceneView& scene, const CpuServices& services,
                  const PathRenderSettings& settings)
    : scene_(scene), services_(services), settings_(settings) {}
  
    void render(const PinholeParams& camera, Film& film) const;
    
private:
  SceneView           scene_;
  const CpuServices&  services_;
  PathRenderSettings  settings_;
};

}