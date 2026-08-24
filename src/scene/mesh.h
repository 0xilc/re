#pragma once

#include <vector>
#include <cstdint>

#include "core/types.h"
#include "scene/mesh_view.h"

namespace re {

struct Mesh {
  std::vector<float3>   positions;
  std::vector<uint32_t> indices;
  std::vector<uint16_t> mat_ids;
  std::vector<float2>   uvs;
};

MeshView make_meshview(const Mesh& mesh);

} // namespace re