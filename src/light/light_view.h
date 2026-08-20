#pragma once

#include "core/platform.h"
#include "core/types.h"
#include "light/light_sampler.h"

namespace re {

struct LightView {
  const LightRef* refs  = nullptr;
  const float*    cdf   = nullptr;
  int             count = 0;
};

// Reverse lookup for MIS: which light is this hit, if any. Linear for now; the
// light sampler replaces it with a per instance offset table.
RE_HD inline int find_light(const LightView& lights, int instance_id, int prim_id) {
  for (int i = 0; i < lights.count; ++i) {
    if (lights.refs[i].instance_id == instance_id && lights.refs[i].prim_id == prim_id) return i;
  }

  return -1;
}

struct LightPick {
  int   index;
  float pdf;
};

RE_HD inline LightPick pick_light(const LightView& lights, float u) {
  if (lights.count <= 0) return LightPick{-1, 0.0f};
  
  int lo = 0;
  int hi = lights.count;
  while (lo < hi) {
    const int mid = (lo + hi) / 2;
    if (lights.cdf[mid + 1] <= u){
      lo = mid + 1;
    } else {
      hi = mid;
    }
  }
  const int index = lo < lights.count ? lo : lights.count - 1;
  return LightPick{index, lights.cdf[index + 1] - lights.cdf[index]};
}

RE_HD inline float pick_light_pdf(const LightView& lights, int index) {
  if (index < 0 || index >= lights.count) return 0.0f;
  return lights.cdf[index + 1] - lights.cdf[index];
}

}