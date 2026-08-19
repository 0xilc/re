#pragma once

#include "accel/brute_accel.h"
#include "camera/pinhole.h"
#include "film/film.h"
#include "scene/mesh.h"
#include "parallel/scheduler.h"

namespace re {

enum class DebugMode { Normals, Lambert }; 

class CpuDebugRenderer {
public:
  CpuDebugRenderer(const MeshView& geometry, const Accel& accel, DebugMode mode, ThreadPool& pool)
    : geometry_(geometry), accel_(accel), mode_(mode), pool_(&pool){}

  void render(const PinholeParams& camera, Film& film) const;

private:
  MeshView geometry_;
  const Accel& accel_;
  DebugMode mode_;
  ThreadPool* pool_;
};

}