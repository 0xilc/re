#pragma once

#include <cgltf.h>

#include "io/gltf/context.h"

namespace re {

constexpr int kInvalidMesh = -1;

int import_mesh(GltfContext& ctx, const cgltf_primitive& prim);

} // namespace re
