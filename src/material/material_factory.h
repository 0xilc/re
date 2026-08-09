#pragma once

#include "core/frame.h"
#include "material/bsdf.h"
#include "core/spectrum.h"
#include "scene/material.h"

namespace re {

inline constexpr float kDielectricF0 = 0.04f;

RE_HD inline Bsdf make_bsdf(const Material& mat, const float3& wo) {
  Bsdf bsdf;

  const Spectrum diffuse_albedo = mat.base_color * (1.0f - mat.metallic);
  const Spectrum f0 = lerp(Spectrum{kDielectricF0, kDielectricF0, kDielectricF0}, mat.base_color, mat.metallic);
  
  Lobe diffuse;
  diffuse.tag = LobeTag::Lambertian;
  diffuse.lambertian.albedo = diffuse_albedo;
  bsdf_add_lobe(bsdf, diffuse, luminance(diffuse_albedo));

  Lobe specular;
  specular.tag = LobeTag::Microfacet;
  specular.microfacet.f0 = f0;
  specular.microfacet.alpha = mat.roughness * mat.roughness;
  const Spectrum fr = fresnel_schlick(f0, local::abs_cos_theta(wo));
  bsdf_add_lobe(bsdf, specular, luminance(fr));
  bsdf_finalize(bsdf);

  return bsdf;
}

}