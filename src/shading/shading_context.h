#pragma once

#include "core/types.h"
#include "core/frame.h"
#include "material/bsdf.h"
#include "integrator/device_services.h"
#include "scene/material.h"
#include "material/bsdf_build.h"
#include "shading/surface_interaction.h"
#include "shading/normal_map.h"

namespace re {

struct ShadingContext {
  Frame frame;
  Bsdf bsdf;
  float3 wo_local;
};

template <Services S>
RE_HD inline ShadingContext make_shading_context(
  const S& services,
  const SurfaceInteraction& si,
  const Material& mat) 
{
  float3 ns = si.ns;

  if (mat.normal_tex != kNoTexture) {
    const Frame tangent_frame = make_frame(si.ns, si.tangent.xyz, si.tangent.w);
    const float4 texel = services.sample_texture(mat.normal_tex, si.uv, si.lod);

    ns = apply_normal_map(texel, mat.normal_scale, tangent_frame);
  }

  const Frame frame = make_frame(ns);
  const float3 wo_local = frame.to_local(si.wo);
  const Bsdf bsdf = make_bsdf(mat, wo_local);
  
  return ShadingContext {
    frame,
    bsdf,
    wo_local
  };
};
}