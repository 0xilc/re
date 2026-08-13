#pragma once

#include "camera/pinhole.h"
#include "film/film.h"
#include "render/cpu/cpu_services.h"
#include "scene/scene_view.h"
#include "light/light_view.h"

namespace re {

enum class IntegratorMode { Naive, Nee, Mis };

struct PathRenderSettings {
  int samples_per_pixel   = 64;
  int max_depth           = 8;
  IntegratorMode mode     = IntegratorMode::Nee;
};

class CpuPathRenderer {
public:
  CpuPathRenderer(const SceneView& scene, const LightView& lights, const CpuServices& services,
                  const PathRenderSettings& settings)
    : scene_(scene), lights_(lights), services_(services), settings_(settings) {}
  
    void render(const PinholeParams& camera, Film& film) const;
    
private:
  SceneView           scene_;
  LightView           lights_;
  const CpuServices&  services_;
  PathRenderSettings  settings_;
};

}