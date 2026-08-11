#pragma once

#include "core/ray.h"
#include "core/spectrum.h"

namespace re {

struct PathState {

  Ray       ray;
  Spectrum  throughput;
  Spectrum  radiance;
  float     pdf_bsdf;
  int       depth;
  bool      specular_bounce;
};

RE_HD inline PathState make_path_state(const Ray& ray) {
  PathState s;
  s.ray             = ray;
  s.throughput      = Spectrum{1.0f, 1.0f, 1.0f};
  s.radiance        = Spectrum{0.0f, 0.0f, 0.0f};
  s.pdf_bsdf        = 0.0f;
  s.depth           = 0;
  s.specular_bounce = true;
  return s;
}

}

