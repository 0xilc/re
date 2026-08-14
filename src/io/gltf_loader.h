#pragma once

#include <string>
#include "scene/scene_builder.h"

namespace re {

bool load_gltf(const std::string& path, Scene& scene);

}