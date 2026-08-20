#pragma once

#include <cgltf.h>
#include <vector>

#include "core/transform.h"
#include "io/gltf/context.h"

namespace re {

// EXT_mesh_gpu_instancing 
bool read_gpu_instances(GltfContext& ctx, const cgltf_node& node, const Transform& world,
                        std::vector<Transform>& out);

} // namespace re
