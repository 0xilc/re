#pragma once
#include <vector>

#include "accel/accel.h"

namespace re {

class EmbreeAccel final : public Accel {
public:
  EmbreeAccel();
  ~EmbreeAccel() override;

  EmbreeAccel(const EmbreeAccel&)             = delete;
  EmbreeAccel& operator=(const EmbreeAccel&)  = delete;

  void build(const SceneGeometryView& geometry) override;
  Hit intersect(const Ray& ray) const override;
  bool occluded(const Ray& ray) const override;

private:
  void release_scenes();

  void* device_ = nullptr;
  void* scene_  = nullptr;

  std::vector<void*> mesh_scenes_;
};

}