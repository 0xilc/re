#pragma once

#include "core/platform.h"
#include "scene/mesh_view.h"
#include "scene/instance.h"
#include "scene/material.h"

namespace re {

struct SceneGeometryView {
  const MeshView* meshes          = nullptr;
  const Instance* instances       = nullptr;
  uint32_t        mesh_count      = 0;
  uint32_t        instance_count  = 0;
};

struct SceneView {
  SceneGeometryView  geometry;
  const Material*    materials = nullptr;
  uint32_t           material_count = 0;

  RE_HD const Material& material_at(uint32_t inst_id, uint32_t prim_id) const {
    uint32_t mesh_id  = geometry.instances[inst_id].mesh_id;
    uint32_t mat_id   = geometry.meshes[mesh_id].mat_ids[prim_id];
    return materials[mat_id];
  }

  RE_HD Triangle triangle_at(uint32_t inst_id, uint32_t prim_id) const {
    const Instance& inst = geometry.instances[inst_id];
    return transform_triangle(inst.to_world,
                              get_triangle(geometry.meshes[inst.mesh_id], prim_id));
  }

  RE_HD float2 uv_at(uint32_t inst_id, uint32_t prim_id, float u, float v) const {
    const Instance& inst = geometry.instances[inst_id];
    return interpolate_uv(geometry.meshes[inst.mesh_id], prim_id, u, v);
  }
};

}