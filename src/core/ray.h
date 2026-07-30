#pragma once

#include "types.h"

namespace re {

struct alignas(32) Ray {
  float3 origin;
  float tmin;
  float3 dir;
  float tmax;
};

}