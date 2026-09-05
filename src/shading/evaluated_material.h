#pragma once

#include "core/platform.h"
#include "core/types.h"
#include "scene/material.h"
#include "shading/surface_interaction.h"

namespace re {

template <typename S>
RE_HD inline EvaluatedMaterial evaluate_material(const Material& mat, const SurfaceInteraction& si, const S& services) {
  EvaluatedMaterial sd = mat.evaluated();

  const float4 base = services.sample_texture(mat.base_color_tex, si.uv, si.lod);
  sd.base_color = mat.base_color * base.xyz;

  const float4 mr = services.sample_texture(mat.metallic_roughness_tex, si.uv, si.lod);
  sd.roughness = mat.roughness * mr.y;
  sd.metallic = mat.metallic * mr.z;
  
  if (mat.is_emissive()) {
    const float4 em = services.sample_texture(mat.emissive_tex, si.uv, si.lod);
    sd.emission = mat.emission * em.xyz;
  }

  return sd;
}

} // namespace re