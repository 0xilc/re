#pragma once

#include <vector>

#include "light/cdf_light_selector.h"
#include "scene/scene_view.h"

namespace re {

struct CdfLightData {
  std::vector<LightRef> refs;
  std::vector<float>    cdf;

  CdfLightSelector selector() const {
    CdfLightSelector s;
    s.refs  = refs.data();
    s.cdf   = cdf.data();
    s.count = static_cast<int>(refs.size());
    return s;
  }
};

CdfLightData build_cdf_lights(const SceneView& scene);

}
