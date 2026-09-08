#pragma once

#include "core/platform.h"
#include "core/types.h"
#include "scene/material.h"
#include "shading/surface_interaction.h"

namespace re {

template <typename S>
RE_HD inline Material evaluate_material(const Material& mat, const SurfaceInteraction& si, const S& services) {
  Material eval = mat;
  const float4 base = services.sample_texture(eval.base_color_tex, si.uv, si.lod);
  eval.base_color = eval.base_color * base.xyz;

  const float4 mr = services.sample_texture(eval.metallic_roughness_tex, si.uv, si.lod);
  eval.roughness = eval.roughness * mr.y;
  eval.metallic = eval.metallic * mr.z;
  
  if (mat.is_emissive()) {
    const float4 em = services.sample_texture(eval.emissive_tex, si.uv, si.lod);
    eval.emission = eval.emission * em.xyz;
  }

  return eval;
}

} // namespace re