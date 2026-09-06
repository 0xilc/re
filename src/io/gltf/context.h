#pragma once

#include <map>
#include <string>
#include <utility>
#include <cgltf.h>
#include <unordered_map>

#include "scene/scene_builder.h"

namespace re {

struct GltfContext {
  const cgltf_data* data  = nullptr;
  Scene*            scene = nullptr;

  std::unordered_map<const cgltf_primitive*, uint32_t> mesh_ids;
  std::map<std::pair<const cgltf_texture*, ColorSpace>, uint32_t> texture_ids;
  std::unordered_map<const cgltf_material*, uint16_t> material_ids;
  
  std::string base_dir;

  uint16_t placeholder_material = 0;
  int errors = 0;
};

} // namespace re
