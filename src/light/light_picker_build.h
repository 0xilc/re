#pragma once

#include <vector>
#include "scene/mesh.h"
#include "scene/material.h"

namespace re {

std::vector<float> build_light_cdf(const MeshView& geometry, 
                                   const Material* materials, 
                                   const std::vector<int>& emissive_prims);

}