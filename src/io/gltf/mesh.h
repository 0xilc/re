#pragma once

#include <cgltf.h>
#include <cstdint>

#include "core/types.h"
#include "io/gltf/context.h"

namespace re {

inline constexpr uint32_t kInvalidMesh = INVALID_ID;

uint32_t import_mesh(GltfContext& ctx, const cgltf_primitive& prim);

} // namespace re
