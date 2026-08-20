#include "io/gltf/accessor.h"

namespace re {

bool read_vec3(const cgltf_accessor* accessor, std::vector<float3>& out) {
  if (accessor == nullptr) return false;

  const size_t count = accessor->count;
  out.resize(count);

  for (size_t i = 0; i < count; ++i) {
    float v[3] = {0.0f, 0.0f, 0.0f};
    if (!cgltf_accessor_read_float(accessor, i, v, 3)) return false;
    out[i] = float3{v[0], v[1], v[2]};
  }

  return true;
}

bool read_vec4(const cgltf_accessor* accessor, std::vector<float4>& out) {
  if (accessor == nullptr) return false;

  const size_t count = accessor->count;
  out.resize(count);

  for (size_t i = 0; i < count; ++i) {
    float v[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    if (!cgltf_accessor_read_float(accessor, i, v, 4)) return false;
    out[i] = float4{v[0], v[1], v[2], v[3]};
  }

  return true;
}

bool read_indices(const cgltf_primitive& prim, size_t vertex_count, std::vector<uint32_t>& out) {
  if (prim.indices != nullptr) {
    out.resize(prim.indices->count);
    for (size_t i = 0; i < prim.indices->count; ++i) {
      out[i] = static_cast<uint32_t>(cgltf_accessor_read_index(prim.indices, i));
    }
    return true;
  }

  out.resize(vertex_count);
  for (size_t i = 0; i < vertex_count; ++i) out[i] = static_cast<uint32_t>(i);

  return true;
}

const cgltf_accessor* find_attribute(const cgltf_primitive& prim, cgltf_attribute_type type) {
  for (size_t i = 0; i < prim.attributes_count; ++i) {
    if (prim.attributes[i].type == type) return prim.attributes[i].data;
  }

  return nullptr;
}

} // namespace re
