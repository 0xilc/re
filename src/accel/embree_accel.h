#pragma once
#include "accel/accel.h"

namespace re {

class EmbreeAccel final : public Accel {
public:
  EmbreeAccel();
  ~EmbreeAccel() override;

  EmbreeAccel(const EmbreeAccel&)             = delete;
  EmbreeAccel& operator=(const EmbreeAccel&)  = delete;

  void build(const GeometryView& geometry) override;
  Hit intersect(const Ray& ray) const override;
  bool occluded(const Ray& ray) const override;

private:
  void* device_ = nullptr;
  void* scene_  = nullptr;

  int triangle_count_ = 0;
};

}