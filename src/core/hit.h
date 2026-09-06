#pragma once

#include "core/platform.h"

namespace re {

inline constexpr int INVALID_ID = -1;

struct Hit {
  float t;
  float u, v;
  int prim_id  = INVALID_ID;
  int inst_id  = INVALID_ID;
};

RE_HD inline bool is_hit(const Hit& h) {
  return h.prim_id != INVALID_ID;
}
}