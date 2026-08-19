#pragma once

#include "accel/accel.h"
#include "core/ray.h"
#include "core/hit.h"

namespace re {

class BruteAccel : public Accel {
public:
  void build(const SceneGeometryView& scene_geometry) override;
  Hit intersect(const Ray& ray) const override;
  bool occluded(const Ray& ray) const override;
  
private:
  SceneGeometryView scene_geometry_;
};

}