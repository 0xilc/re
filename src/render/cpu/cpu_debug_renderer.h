#pragma once

#include "camera/pinhole.h"
#include "film/film.h"
#include "scene/scene_view.h"
#include "parallel/thread_pool.h"
#include "accel/accel.h"

namespace re {

enum class DebugMode { Normals, Lambert }; 

class CpuDebugRenderer {
public:
  CpuDebugRenderer(const SceneGeometryView& geometry, const Accel& accel, DebugMode mode, ThreadPool& pool)
    : geometry_(geometry), accel_(accel), mode_(mode), pool_(&pool){}

  void render(const PinholeParams& camera, Film& film) const;

private:
  SceneGeometryView geometry_;
  const Accel& accel_;
  DebugMode mode_;
  ThreadPool* pool_;
};

}