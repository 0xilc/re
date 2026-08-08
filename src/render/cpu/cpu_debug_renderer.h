#pragma once

#include "accel/brute_accel.h"
#include "camera/pinhole.h"
#include "film/film.h"
#include "scene/geometry.h"

namespace re {

enum class DebugMode { Normals, Lambert }; 

class CpuDebugRenderer {
public:
  CpuDebugRenderer(const GeometryView& geometry, const BruteAccel& accel, DebugMode mode)
    : geometry_(geometry), accel_(accel), mode_(mode) {}

  void render(const PinholeParams& camera, Film& film) const;

private:
  GeometryView geometry_;
  const BruteAccel& accel_;
  DebugMode mode_;
};

}