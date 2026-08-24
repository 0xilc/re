#pragma once

#include "core/platform.h"
#include "scene/mesh_view.h"
#include "scene/instance.h"
#include "scene/material.h"
#include "scene/texture_view.h"

namespace re {

struct SceneGeometryView {
  const MeshView* meshes          = nullptr;
  const Instance* instances       = nullptr;
  size_t          mesh_count      = 0;
  size_t          instance_count  = 0;
};

struct SceneView {
  SceneGeometryView  geometry;
  const Material*    materials = nullptr;
  int                material_count = 0;
  const TextureView* textures = nullptr;
  int                texture_count = 0;

  RE_HD const Material& material_at(int inst_id, int prim_id) const {
    int mesh_id = geometry.instances[inst_id].mesh_id;
    int mat_id = geometry.meshes[mesh_id].mat_ids[prim_id];
    return materials[mat_id];
  }

  RE_HD Triangle triangle_at(int inst_id, int prim_id) const {
    const Instance& inst = geometry.instances[inst_id];
    return transform_triangle(inst.to_world,
                              get_triangle(geometry.meshes[inst.mesh_id], prim_id));
  }

  RE_HD float2 uv_at(int inst_id, int prim_id, float u, float v) const {
    const Instance& inst = geometry.instances[inst_id];
    return interpolate_uv(geometry.meshes[inst.mesh_id], prim_id, u, v);
  }
};

}