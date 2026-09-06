#include "io/gltf/loader.h"

#include <cgltf.h>
#include <filesystem>

#include "core/log.h"
#include "core/transform.h"
#include "io/gltf/context.h"
#include "io/gltf/instancing.h"
#include "io/gltf/mesh.h"

namespace re {
namespace {

Transform from_cgltf(const float m[16]) {
  Transform t;
  t.r[0] = float4(m[0], m[4], m[8],  m[12]);
  t.r[1] = float4(m[1], m[5], m[9],  m[13]);
  t.r[2] = float4(m[2], m[6], m[10], m[14]);
  return t;
}

void import_node(GltfContext& ctx, const cgltf_node& node) {
  if (node.mesh == nullptr) return;

  float raw[16];
  cgltf_node_transform_world(&node, raw);
  const Transform world = from_cgltf(raw);

  std::vector<Transform> transforms;
  if (!read_gpu_instances(ctx, node, world, transforms)) transforms.push_back(world);

  for (size_t p = 0; p < node.mesh->primitives_count; ++p) {
    const cgltf_primitive& prim = node.mesh->primitives[p];

    const uint32_t mesh_id = import_mesh(ctx, prim);
    if (mesh_id == kInvalidMesh) continue;

    for (const Transform& to_world : transforms) {
      ctx.scene->add_instance(to_world, mesh_id);
    }
  }
}

cgltf_data* open_gltf(const std::string& path) {
  cgltf_options options{};
  cgltf_data* data = nullptr;

  if (cgltf_parse_file(&options, path.c_str(), &data) != cgltf_result_success) {
    log_error("gltf: could not parse {}", path);
    return nullptr;
  }

  if (cgltf_load_buffers(&options, data, path.c_str()) != cgltf_result_success) {
    log_error("gltf: could not load buffers for {}", path);
    cgltf_free(data);
    return nullptr;
  }

  if (cgltf_validate(data) != cgltf_result_success) {
    log_error("gltf: {} failed validation", path);
    cgltf_free(data);
    return nullptr;
  }

  return data;
}

} // namespace

bool load_gltf(const std::string& path, Scene& scene) {
  cgltf_data* data = open_gltf(path);
  if (data == nullptr) return false;

  GltfContext ctx;
  ctx.data  = data;
  ctx.scene = &scene;
  ctx.base_dir = std::filesystem::path(path).parent_path().string();
  ctx.placeholder_material =
      scene.add_material(make_material({0.75f, 0.75f, 0.75f}, 0.0f, 0.9f));

  const size_t meshes_before    = scene.meshes.size();
  const size_t instances_before = scene.instances.size();

  for (size_t i = 0; i < data->nodes_count; ++i) import_node(ctx, data->nodes[i]);

  const size_t meshes_added    = scene.meshes.size() - meshes_before;
  const size_t instances_added = scene.instances.size() - instances_before;

  size_t triangles_added = 0;
  size_t vertices_added  = 0;
  for (size_t i = meshes_before; i < scene.meshes.size(); ++i) {
    triangles_added += scene.meshes[i].mat_ids.size();
    vertices_added  += scene.meshes[i].positions.size();
  }

  log_info("gltf: loaded {} ({} meshes, {} instances, {} triangles, {} vertices)", path,
           meshes_added, instances_added, triangles_added, vertices_added);

  if (meshes_added == 0) log_error("gltf: {} contributed no geometry", path);
  if (ctx.errors > 0) log_error("gltf: {} primitives in {} were skipped", ctx.errors, path);

  cgltf_free(data);
  return ctx.errors == 0 && meshes_added > 0;
}

} // namespace re
