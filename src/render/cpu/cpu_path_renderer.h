#pragma once

#include "camera/pinhole.h"
#include "film/film.h"
#include "render/cpu/cpu_services.h"
#include "scene/scene_view.h"
#include "light/cdf_light_selector.h"
#include "parallel/thread_pool.h"
#include "config/render_settings.h"

namespace re {

class CpuPathRenderer {
public:
  CpuPathRenderer(const SceneView& scene, const CdfLightSelector& lights, const CpuServices& services,
                  ThreadPool& pool, const PathTracerSettings& settings)
    : scene_(scene), lights_(lights), services_(services), pool_(&pool), settings_(settings) {}
  
    void render(const PinholeParams& camera, Film& film) const;
    
private:
  SceneView           scene_;
  CdfLightSelector           lights_;
  const CpuServices&  services_;
  ThreadPool*         pool_;
  PathTracerSettings  settings_;
};

}