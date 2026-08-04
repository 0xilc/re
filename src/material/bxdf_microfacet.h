#pragma once

#include "core/spectrum.h"
#include "material/bxdf.h"
#include "material/fresnel.h"
#include "material/microfacet_dist.h"

namespace re {

struct MicrofacetLobe {
  Spectrum f0;
  float alpha;
};

RE_HD inline Spectrum microfacet_eval(const MicrofacetLobe& lobe, const float3& wo, const float3& wi) {
  if (!local::same_hemisphere(wo, wi)) return make_empty_spectrum();

  const float cos_o = local::abs_cos_theta(wo);
  const float cos_i = local::abs_cos_theta(wi);
  if (cos_o == 0.0f || cos_i == 0.0f) return make_empty_spectrum();

  float3 wm = wo + wi;
  const float len = length(wm);
  if (len == 0.0f) return make_empty_spectrum();
  wm = wm * (1.0f / len);
  if (wm.z < 0.0f) wm = -wm;

  const float alpha = clamp_alpha(lobe.alpha);
  const float d = ggx_d(wm, alpha);
  const float g = ggx_g2(wo, wi, alpha);
  const Spectrum f = fresnel_schlick(lobe.f0, fabsf(dot(wo, wm)));

  return f * (d * g / (4.0f * cos_o * cos_i));
}

RE_HD inline float microfacet_pdf(const MicrofacetLobe& lobe, const float3& wo, const float3& wi) {
  if (!local::same_hemisphere(wo, wi)) return 0.0f;

  float3 wm = wo + wi;
  const float len = length(wm);
  if (len == 0.0f) return 0.0f;
  wm = wm * (1.0f / len);
  if (wm.z < 0.0f) wm = -wm;
  
  const float alpha = clamp_alpha(lobe.alpha);
  const float dot_wo_wm = dot(wo, wm);
  if (dot_wo_wm <= 0.0f) return 0.0f;
  return ggx_vndf_pdf(wo, wm, alpha) / (4.0f * dot_wo_wm);
}

RE_HD inline BsdfSample microfacet_sample(const MicrofacetLobe& lobe, const float3& wo, float2 u) {
    if (wo.z == 0.0f) return invalid_sample();

    const float  alpha = clamp_alpha(lobe.alpha);
    const bool   flip  = wo.z < 0.0f;
    const float3 wo_u  = flip ? -wo : wo;

    const float3 wm   = ggx_sample_vndf(wo_u, alpha, u);
    const float3 wi_u = reflect(wo_u, wm);
    if (wi_u.z <= 0.0f) return invalid_sample();

    const float cos_o = wo_u.z;
    const float cos_i = wi_u.z;

    const float d        = ggx_d(wm, alpha);
    const float lambda_o = ggx_lambda(wo_u, alpha);
    const float lambda_i = ggx_lambda(wi_u, alpha);
    const float g1       = 1.0f / (1.0f + lambda_o);
    const float g2       = 1.0f / (1.0f + lambda_o + lambda_i);

    const Spectrum fr = fresnel_schlick(lobe.f0, dot(wo_u, wm));

    BsdfSample s;
    s.wi  = flip ? -wi_u : wi_u;
    s.f   = fr * (d * g2 / (4.0f * cos_o * cos_i));
    s.pdf = g1 * d / (4.0f * cos_o);
    return s;
}
}