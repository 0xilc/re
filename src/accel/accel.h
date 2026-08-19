#pragma once

#include "core/hit.h"
#include "core/ray.h"
#include "scene/scene_view.h"

namespace re {

class Accel {
public:
  virtual ~Accel() = default;
  virtual void build(const SceneGeometryView& geometry) = 0;
  virtual Hit intersect(const Ray& ray) const = 0;
  virtual bool occluded(const Ray& ray) const = 0;
};

}