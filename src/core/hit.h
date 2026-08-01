#pragma once

#include "core/platform.h"

namespace re {

constexpr int INVALID_ID = -1;

struct Hit {
  float t;
  float u, v;
  int primitive_id  = INVALID_ID;
  int instance_id   = INVALID_ID;
};

RE_HD inline bool is_hit(const Hit& h) {
  return h.primitive_id != INVALID_ID;
}
}