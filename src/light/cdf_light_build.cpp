#include "light/cdf_light_build.h"

#include "core/spectrum.h"

namespace re {

CdfLightData build_cdf_lights(const SceneView& scene) {
  CdfLightData data;
  std::vector<float> weights;

  const int instance_count = static_cast<int>(scene.geometry.instance_count);
  for (int inst_id = 0; inst_id < instance_count; ++inst_id) {
    const Instance& inst = scene.geometry.instances[inst_id];
    const MeshView& mesh = scene.geometry.meshes[inst.mesh_id];

    for (int prim_id = 0; prim_id < mesh.triangle_count; ++prim_id) {
      const Material& mat = scene.material_at(inst_id, prim_id);
      if (!mat.is_emissive()) continue;

      const float area = scene.triangle_at(inst_id, prim_id).area();
      if (!(area > 0.0f)) continue;

      data.refs.push_back(LightRef{inst_id, prim_id});
      weights.push_back(luminance(mat.emission) * area);
    }
  }

  const size_t count = data.refs.size();
  data.cdf.assign(count + 1, 0.0f);
  if (count == 0) return data;

  float total = 0.0f;
  for (size_t i = 0; i < count; ++i) {
    total += weights[i];
    data.cdf[i + 1] = total;
  }

  if (total <= 0.0f) {
    // Emissive prims with zero luminance should not exist. Fall back to uniform
    // rather than dividing by zero.
    for (size_t i = 0; i <= count; ++i) {
      data.cdf[i] = static_cast<float>(i) / static_cast<float>(count);
    }

    return data;
  }

  const float inv = 1.0f / total;
  for (size_t i = 1; i <= count; ++i) data.cdf[i] *= inv;
  data.cdf[count] = 1.0f;

  return data;
}

}
