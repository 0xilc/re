#pragma once

#include <cgltf.h>
#include <unordered_map>

#include "scene/scene_builder.h"

namespace re {

struct GltfContext {
  const cgltf_data* data  = nullptr;
  Scene*            scene = nullptr;

  std::unordered_map<const cgltf_primitive*, int> mesh_ids;

  uint16_t placeholder_material = 0;

  int errors = 0;
};

} // namespace re
