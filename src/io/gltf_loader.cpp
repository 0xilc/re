#include "io/gltf_loader.h"

#include <cgltf.h>
#include <cstring>
#include <utility>
#include <vector>

#include "core/log.h"
#include "core/transform.h"

namespace re {
namespace {
Transform from_cgltf(const float m[16]) {
    Transform t;
    t.r[0] = float4(m[0], m[4], m[8],  m[12]);
    t.r[1] = float4(m[1], m[5], m[9],  m[13]);
    t.r[2] = float4(m[2], m[6], m[10], m[14]);
    return t;
}

bool read_positions(const cgltf_accessor* accessor, std::vector<float3>& out) {
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

void add_primitive(const cgltf_primitive& prim, const Transform& world, uint16_t material_id, Scene& scene) {
  if (prim.type != cgltf_primitive_type_triangles) return;

  Mesh mesh;
  if (!read_positions(find_attribute(prim, cgltf_attribute_type_position), mesh.positions)) {
    log_error("gltf: primitive has no readable POSITION attribute");
    return;
  }

  std::vector<uint32_t> indices;
  if (!read_indices(prim, mesh.positions.size(), indices)) {
    log_error("gltf: could not read indices");
    return;
  }
  if (indices.size() % 3 != 0) {
    log_error("gltf: index count {} is not a multiple of three", indices.size());
    return;
  }
  if (indices.empty()) return;

  for (float3& p : mesh.positions) p = transform_point(world, p);

  const bool flip = is_mirrored(world);
  mesh.indices.reserve(indices.size());
  mesh.mat_ids.assign(indices.size() / 3, material_id);
  for (size_t i = 0; i + 2 < indices.size(); i += 3) {
    mesh.indices.push_back(indices[i]);
    mesh.indices.push_back(indices[i + (flip ? 2 : 1)]);
    mesh.indices.push_back(indices[i + (flip ? 1 : 2)]);
  }

  scene.add_mesh(std::move(mesh));
}
} // namespace

bool load_gltf(const std::string& path, Scene& scene) {
  cgltf_options options{};
  cgltf_data* data = nullptr;

  if (cgltf_parse_file(&options, path.c_str(), &data) != cgltf_result_success){
    log_error("gltf: could not parse {}", path);
    return false;
  }

  if (cgltf_load_buffers(&options, data, path.c_str()) != cgltf_result_success) {
    log_error("gltf: could not load buffers for {}", path);
    cgltf_free(data);
    return false;
  }

  if (cgltf_validate(data) != cgltf_result_success) {
    log_error("gltf: {} failed validation", path);
    cgltf_free(data);
    return false;
  }

  // For now, a placeholder material
  const uint16_t placeholder = scene.add_material(make_material({0.75f, 0.75f, 0.75f}, 0.0f, 0.9f));
  
  const size_t meshes_before = scene.meshes.size();

  for (size_t i = 0; i < data->nodes_count; ++i) {
    const cgltf_node& node = data->nodes[i];
    if (node.mesh == nullptr) continue;

    float raw[16];
    cgltf_node_transform_world(&node, raw);
    const Transform world = from_cgltf(raw);
    
    for (size_t p = 0; p < node.mesh->primitives_count; ++p) {
      add_primitive(node.mesh->primitives[p], world, placeholder, scene);
    }
  }

  const size_t meshes_added = scene.meshes.size() - meshes_before;
  size_t triangles_added = 0;
  size_t vertices_added  = 0;
  for (size_t i = meshes_before; i < scene.meshes.size(); ++i) {
    triangles_added += scene.meshes[i].mat_ids.size();
    vertices_added  += scene.meshes[i].positions.size();
  }

  log_info("gltf: loaded {} ({} meshes, {} triangles, {} vertices)", path, meshes_added,
           triangles_added, vertices_added);

  cgltf_free(data);
  return meshes_added > 0;
}

} // namespace re