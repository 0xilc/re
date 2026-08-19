#pragma once

#include <cstdint>
#include "core/transform.h"

namespace re {

struct Instance {
  Transform to_world;
  Transform to_local;
  uint32_t  mesh_id;
};

inline Instance make_instance(const Transform& to_world, uint32_t mesh_id) {
  Instance inst;
  inst.to_world = to_world;
  inst.to_local = inverse(to_world);
  inst.mesh_id = mesh_id;
  return inst;
}

} // namespace re

