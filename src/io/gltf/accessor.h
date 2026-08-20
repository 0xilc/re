#pragma once

#include <cgltf.h>
#include <cstdint>
#include <vector>

#include "core/types.h"

namespace re {

bool read_vec3(const cgltf_accessor* accessor, std::vector<float3>& out);

bool read_vec4(const cgltf_accessor* accessor, std::vector<float4>& out);

bool read_indices(const cgltf_primitive& prim, size_t vertex_count, std::vector<uint32_t>& out);

const cgltf_accessor* find_attribute(const cgltf_primitive& prim, cgltf_attribute_type type);

} // namespace re
