#pragma once

namespace re {

constexpr int INVALID_ID = -1;

struct alignas(32) Hit {
  float t;
  float u, v;
  int primitive_id = INVALID_ID;
  int instance_id;
  int _pad[3];
};

inline bool is_hit(const Hit& h) {
  return h.primitive_id != INVALID_ID;
}
}