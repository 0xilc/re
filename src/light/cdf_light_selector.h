#pragma once

#include "core/hit.h"
#include "core/platform.h"
#include "light/light_selector.h"

namespace re {

struct CdfLightSelector {
  const LightRef* refs  = nullptr;
  const float*    cdf   = nullptr;
  int             count = 0;

  RE_HD LightPick pick(const LightSampleContext&, float u) const {
    if (count <= 0) return LightPick{LightRef{INVALID_ID, INVALID_ID}, 0.0f};

    int lo = 0;
    int hi = count;
    while (lo < hi) {
      const int mid = (lo + hi) / 2;
      if (cdf[mid + 1] <= u) {
        lo = mid + 1;
      } else {
        hi = mid;
      }
    }

    const int index = lo < count ? lo : count - 1;
    return LightPick{refs[index], cdf[index + 1] - cdf[index]};
  }

  RE_HD float pmf(const LightSampleContext&, LightRef ref) const {
    for (int i = 0; i < count; ++i) {
      if (refs[i].instance_id == ref.instance_id && refs[i].prim_id == ref.prim_id) {
        return cdf[i + 1] - cdf[i];
      }
    }

    return 0.0f;
  }
};

static_assert(LightSelector<CdfLightSelector>,
              "CdfLightSelector must stay usable as a light sampling strategy");

}