#pragma once

#include <cstdint>
#include <vector>

#include "scene/mesh.h"
#include "scene/mesh_view.h"
#include "scene/material.h"
#include "scene/instance.h"
#include "scene/scene_view.h"
#include "scene/texture_desc.h"

namespace re {
struct Scene {
  std::vector<Mesh> meshes;
  std::vector<MeshView> mesh_views;
  std::vector<Instance> instances;
  std::vector<Material> materials;
  std::vector<TextureDesc> texture_descs;

  const Material* material_table() const { return materials.data(); }

  uint32_t add_mesh(Mesh mesh);
  
  uint32_t add_instance(Transform to_world, uint32_t mesh_id);

  uint16_t add_material(const Material& material);

  void add_quad(const float3& p0, const float3& p1, const float3& p2, const float3& p3,
                uint16_t material_id);

  void add_box(const float3& lo, const float3& hi, uint16_t material_id);

  SceneGeometryView make_geometry_view() const;
};

Scene make_cornell_scene();

}  // namespace re