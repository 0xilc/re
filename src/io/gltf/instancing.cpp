#include "io/gltf/instancing.h"

#include <cstring>

#include "core/log.h"
#include "io/gltf/accessor.h"

namespace re {
namespace {

const cgltf_accessor* find_instance_attribute(const cgltf_mesh_gpu_instancing& gpu,
                                              const char* name) {
  for (size_t i = 0; i < gpu.attributes_count; ++i) {
    if (gpu.attributes[i].name != nullptr && std::strcmp(gpu.attributes[i].name, name) == 0) {
      return gpu.attributes[i].data;
    }
  }

  return nullptr;
}

size_t instance_count(const cgltf_mesh_gpu_instancing& gpu) {
  size_t count = 0;
  for (size_t i = 0; i < gpu.attributes_count; ++i) {
    if (gpu.attributes[i].data != nullptr) count = std::max(count, gpu.attributes[i].data->count);
  }

  return count;
}

} // namespace

bool read_gpu_instances(GltfContext& ctx, const cgltf_node& node, const Transform& world,
                        std::vector<Transform>& out) {
  if (!node.has_mesh_gpu_instancing) return false;

  const cgltf_mesh_gpu_instancing& gpu = node.mesh_gpu_instancing;
  const size_t count = instance_count(gpu);
  if (count == 0) return true;

  std::vector<float3> translations;
  std::vector<float4> rotations;
  std::vector<float3> scales;

  const cgltf_accessor* t = find_instance_attribute(gpu, "TRANSLATION");
  const cgltf_accessor* r = find_instance_attribute(gpu, "ROTATION");
  const cgltf_accessor* s = find_instance_attribute(gpu, "SCALE");

  if (t != nullptr && !read_vec3(t, translations)) {
    log_error("gltf: could not read EXT_mesh_gpu_instancing TRANSLATION");
    ++ctx.errors;
    return true;
  }
  if (r != nullptr && !read_vec4(r, rotations)) {
    log_error("gltf: could not read EXT_mesh_gpu_instancing ROTATION");
    ++ctx.errors;
    return true;
  }
  if (s != nullptr && !read_vec3(s, scales)) {
    log_error("gltf: could not read EXT_mesh_gpu_instancing SCALE");
    ++ctx.errors;
    return true;
  }

  out.reserve(out.size() + count);
  for (size_t i = 0; i < count; ++i) {
    const float3 ti = i < translations.size() ? translations[i] : float3{0.0f, 0.0f, 0.0f};
    const float4 ri = i < rotations.size() ? rotations[i] : float4{0.0f, 0.0f, 0.0f, 1.0f};
    const float3 si = i < scales.size() ? scales[i] : float3{1.0f, 1.0f, 1.0f};

    out.push_back(compose(world, make_transform(ti, ri, si)));
  }

  return true;
}

} // namespace re
