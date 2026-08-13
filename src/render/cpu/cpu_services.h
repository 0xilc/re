#pragma once

#include "accel/accel.h"
#include "integrator/device_services.h"

namespace re {

class CpuServices {
public:
  explicit CpuServices(const Accel& accel) : accel_(accel) {}

  Hit intersect(const Ray& ray) const { 
    return accel_.intersect(ray);
  }

  bool occluded(const Ray& ray) const { 
    return accel_.occluded(ray);
  }

private:
  const Accel& accel_;
};

static_assert(Services<CpuServices>, "CpuServices must satisfy the Services concept.");
}