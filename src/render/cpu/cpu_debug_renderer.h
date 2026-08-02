#pragma once

#include "accel/brute_accel.h"
#include "camera/pinhole.h"
#include "film/film.h"
#include "scene/geometry.h"

namespace re {

class CpuDebugRenderer {
public:
  CpuDebugRenderer(const GeometryView& geometry, const BruteAccel& accel)
    : geometry_(geometry), accel_(accel) {}

  void render(const PinholeParams& camera, Film& film) const;

private:
  GeometryView geometry_;
  const BruteAccel& accel_;
};

}