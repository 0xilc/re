#include "io/gltf/mesh.h"

#include <utility>

#include "core/log.h"
#include "io/gltf/accessor.h"

namespace re {
namespace {

int load_primitive(GltfContext& ctx, const cgltf_primitive& prim) {
  if (prim.type != cgltf_primitive_type_triangles) return kInvalidMesh;

  Mesh mesh;
  if (!read_vec3(find_attribute(prim, cgltf_attribute_type_position), mesh.positions)) {
    log_error("gltf: primitive has no readable POSITION attribute");
    ++ctx.errors;
    return kInvalidMesh;
  }

  if (!read_vec3(find_attribute(prim, cgltf_attribute_type_normal), mesh.normals)) {
    mesh.normals.clear();
  }

  if (!read_vec2(find_attribute(prim, cgltf_attribute_type_texcoord), mesh.uvs)) {
    mesh.uvs.clear();
  }

  if (!mesh.normals.empty() && mesh.normals.size() != mesh.positions.size()) {
    log_error("gltf: NORMAL count {} does not match POSITION count {}", mesh.normals.size(),
              mesh.positions.size());
    mesh.normals.clear();
  }

  if (!mesh.uvs.empty() && mesh.uvs.size() != mesh.positions.size()) {
    log_error("gltf: TEXCOORD_0 count {} does not match POSITION count {}", mesh.uvs.size(),
              mesh.positions.size());
    mesh.uvs.clear();
  }

  if (!read_indices(prim, mesh.positions.size(), mesh.indices)) {
    log_error("gltf: could not read indices");
    ++ctx.errors;
    return kInvalidMesh;
  }

  if (mesh.indices.size() % 3 != 0) {
    log_error("gltf: index count {} is not a multiple of three", mesh.indices.size());
    ++ctx.errors;
    return kInvalidMesh;
  }
  
  if (mesh.indices.empty()) return kInvalidMesh;

  mesh.mat_ids.assign(mesh.indices.size() / 3, ctx.placeholder_material);

  return static_cast<int>(ctx.scene->add_mesh(std::move(mesh)));
}

} // namespace

int import_mesh(GltfContext& ctx, const cgltf_primitive& prim) {
  const auto it = ctx.mesh_ids.find(&prim);
  if (it != ctx.mesh_ids.end()) return it->second;

  const int mesh_id = load_primitive(ctx, prim);
  ctx.mesh_ids[&prim] = mesh_id;

  return mesh_id;
}

} // namespace re
