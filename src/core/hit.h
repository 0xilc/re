#pragma once

namespace re {

struct alignas(32) Hit {
  float t;
  float u, v;
  int primitive_id;
  int instance_id;
  int _pad[3];
};

constexpr float HIT_MISS = -1.0f;
}