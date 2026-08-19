#pragma once

#include "core/platform.h"
#include "scene/mesh_view.h"
#include "scene/instance.h"
#include "scene/material.h"

namespace re {

struct SceneView {
  MeshView          geometry;
  const Material*   materials = nullptr;
  int               material_count = 0;

  RE_HD const Material& material_at(int primitive_id) const {
    return materials[geometry.get_material_id(primitive_id)];
  }
};

struct SceneGeometryView {
  const MeshView* meshes          = nullptr;
  const Instance* instances       = nullptr;
  size_t          mesh_count      = 0;
  size_t          instance_count  = 0;
};

}