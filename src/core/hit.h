#pragma once

#include <cstdint>

#include "core/platform.h"
#include "core/types.h"

namespace re {

struct Hit {
  float t;
  float u, v;
  uint32_t prim_id  = INVALID_ID;
  uint32_t inst_id  = INVALID_ID;
};

RE_HD inline bool is_hit(const Hit& h) {
  return h.prim_id != INVALID_ID;
}
}