#pragma once

#include "camera/pinhole.h"
#include "film/film.h"
#include "render/cpu/cpu_services.h"
#include "scene/scene_view.h"
#include "light/light_view.h"
#include "parallel/thread_pool.h"
#include "config/render_settings.h"

namespace re {

class CpuPathRenderer {
public:
  CpuPathRenderer(const SceneView& scene, const LightView& lights, const CpuServices& services,
                  ThreadPool& pool, const PathTracerSettings& settings)
    : scene_(scene), lights_(lights), services_(services), pool_(&pool), settings_(settings) {}
  
    void render(const PinholeParams& camera, Film& film) const;
    
private:
  SceneView           scene_;
  LightView           lights_;
  const CpuServices&  services_;
  ThreadPool*         pool_;
  PathTracerSettings  settings_;
};

}