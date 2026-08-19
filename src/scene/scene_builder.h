#pragma once

#include <cstdint>
#include <vector>

#include "scene/mesh.h"
#include "scene/mesh_view.h"
#include "scene/material.h"
#include "scene/instance.h"
#include "light/light_view.h"

namespace re {
struct Scene {
  std::vector<Mesh> meshes;
  std::vector<Instance> instances;
  std::vector<Material> materials;
  std::vector<float> light_cdf;
  std::vector<int> emissive_prims;
  std::vector<int> prim_to_light;

  MeshView geometry() const {
    return make_meshview(meshes[0]);
  }

  LightView light_view() const {
    LightView l;
    l.prims = emissive_prims.data();
    l.cdf = light_cdf.data();
    l.prim_to_light = prim_to_light.data();
    l.count = static_cast<int>(emissive_prims.size());
    
    return l;
  }

  const Material* material_table() const { return materials.data(); }

  uint32_t add_mesh(Mesh mesh);
  
  uint32_t add_instance(Transform to_world, uint32_t mesh_id);

  uint16_t add_material(const Material& material);

  void add_quad(const float3& p0, const float3& p1, const float3& p2, const float3& p3,
                uint16_t material_id);

  void add_box(const float3& lo, const float3& hi, uint16_t material_id);

  void finalize();
};

Scene make_cornell_scene();

}  // namespace re