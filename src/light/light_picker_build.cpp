#include "light/light_picker_build.h"

#include "core/spectrum.h"

namespace re {

std::vector<float> build_light_cdf(const GeometryView& geometry, 
                                   const Material* materials,
                                   const std::vector<int>& emissive_prims) 
{
  const int count = static_cast<int>(emissive_prims.size());
  std::vector<float> cdf(static_cast<size_t>(count) + 1, 0.0f);
  if (count == 0) return cdf;

  float total = 0.0f;
  for (int i = 0; i < count; ++i) {
    const int prim = emissive_prims[static_cast<size_t>(i)];
    const Triangle tri = get_triangle(geometry, prim);
    const Material& mat = materials[geometry.get_material_id(prim)];
    
    total += luminance(mat.emission) * tri.area();
    cdf[static_cast<size_t>(i) + 1] = total;
  }

  if (total <= 0.0f) {
    // Emissive prims with zero luminance should not exist,
    // instead of error, return uniform prob for each prim.
    for (int i = 0; i <= count; ++i) {
      cdf[static_cast<size_t>(i)] = static_cast<float>(i) / static_cast<float>(count);
    }
    
    return cdf;
  }

  const float inv = 1.0f / total;
  for (int i = 1; i <= count; ++i) {
    cdf[static_cast<size_t>(i)] *= inv;
  }

  cdf[static_cast<size_t>(count)] = 1.0f;
  return cdf;
}

}
