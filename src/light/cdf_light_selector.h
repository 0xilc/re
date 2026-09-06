#pragma once

#include <cstdint>

#include "core/hit.h"
#include "core/platform.h"
#include "light/light_selector.h"

namespace re {

RE_HD inline uint64_t light_key(LightRef ref) {
  return (static_cast<uint64_t>(static_cast<uint32_t>(ref.inst_id)) << 32) |
         static_cast<uint32_t>(ref.prim_id);
}

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
    const uint64_t key = light_key(ref);

    int lo = 0;
    int hi = count;
    while (lo < hi) {
      const int mid = (lo + hi) / 2;
      if (light_key(refs[mid]) < key) {
        lo = mid + 1;
      } else {
        hi = mid;
      }
    }

    if (lo >= count || light_key(refs[lo]) != key) return 0.0f;
    return cdf[lo + 1] - cdf[lo];
  }
};

static_assert(LightSelector<CdfLightSelector>,
              "CdfLightSelector must stay usable as a light sampling strategy");

}