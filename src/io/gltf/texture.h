#pragma once

#include <cgltf.h>
#include <cstdint>

#include "core/color.h"
#include "io/gltf/context.h"

namespace re {

uint32_t import_texture(GltfContext& ctx, const cgltf_texture_view& view, ColorSpace space, bool is_normal_map = false);

} // namespace re