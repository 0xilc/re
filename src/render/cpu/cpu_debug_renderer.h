#pragma once

#include "camera/pinhole.h"
#include "film/film.h"
#include "scene/scene_view.h"
#include "parallel/thread_pool.h"
#include "render/cpu/cpu_services.h"

namespace re {

enum class DebugMode { UV, Normals, Lambert, BaseColor }; 

class CpuDebugRenderer {
public:
  CpuDebugRenderer(const SceneView& scene, const CpuServices& services, DebugMode mode, ThreadPool& pool)
    : scene_(scene), services_(services), mode_(mode), pool_(&pool){}

  void render(const PinholeParams& camera, Film& film) const;

private:
  SceneView           scene_;
  const CpuServices&  services_;
  DebugMode           mode_;
  ThreadPool*         pool_;
};

}