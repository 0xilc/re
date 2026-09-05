#pragma once

#include <cgltf.h>
#include <cstdint>

#include "io/gltf/context.h"

namespace re {

uint16_t import_material(GltfContext& ctx, const cgltf_material* material);

} // namespace re