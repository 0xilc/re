#pragma once

#include <cstdint>
#include "core/platform.h"
#include "core/types.h"


namespace re {

RE_HD inline uint32_t pcg_hash(uint32_t x) {
    uint32_t state = x * 747796405u + 2891336453u;
    uint32_t word  = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

struct Sampler {
  RE_HD Sampler(int px, int py, int sample_index) {
    const uint32_t pixel =  (static_cast<uint32_t>(px) & 0xFFFFu) |
                            (static_cast<uint32_t>(py) << 16);
    base = pcg_hash(pixel ^ pcg_hash(static_cast<uint32_t>(sample_index)));
    dimension = 0;
  }

  RE_HD float next_1d() { 
    return to_float01(pcg_hash(base + dimension++));
  }

  RE_HD float2 next_2d() {
    const uint32_t d = dimension;
    dimension += 2;
    return float2{
      to_float01(pcg_hash(base + d)),
      to_float01(pcg_hash(base + d + 1u))
    };
  }

  RE_HD void skip_1d() { ++dimension; }
  RE_HD void skip_2d() { dimension += 2; }

  uint32_t base;
  uint32_t dimension;

private:
  RE_HD static float to_float01(uint32_t bits) {
    return static_cast<float>(bits >> 8) * 0x1p-24f;
  }
};

}

