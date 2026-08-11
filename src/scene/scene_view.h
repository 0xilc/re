#pragma once

#include "core/platform.h"
#include "scene/geometry.h"
#include "scene/material.h"

namespace re {

struct SceneView {
  GeometryView      geometry;
  const Material*   materials = nullptr;
  int               material_count = 0;

  RE_HD const Material& material_at(int primitive_id) const {
    return materials[geometry.get_material_id(primitive_id)];
  }
};

}