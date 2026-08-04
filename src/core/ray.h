#pragma once

#include "core/types.h"
#include "core/math.h"

namespace re {

struct alignas(32) Ray {
  float3 origin;
  float tmin = 0.0f;
  float3 dir;
  float tmax = kInf;
};

}