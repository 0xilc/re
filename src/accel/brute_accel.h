#pragma once

#include "accel/accel.h"
#include "scene/mesh.h"
#include "core/ray.h"
#include "core/hit.h"

namespace re {

class BruteAccel : public Accel {
public:
  void build(const MeshView& mesh) override;
  Hit intersect(const Ray& ray) const override;
  bool occluded(const Ray& ray) const override;
  
private:
  MeshView mesh_{};
};

}