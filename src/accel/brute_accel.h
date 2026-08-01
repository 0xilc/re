#pragma once

#include "scene/geometry.h"
#include "core/ray.h"
#include "core/hit.h"

namespace re {

class BruteAccel {
public:
  void build(const GeometryView& mesh);
  Hit intersect(const Ray& ray) const;
  
private:
  GeometryView mesh_{};
};

}