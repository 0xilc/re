#pragma once

#include "core/frame.h"
#include "core/spectrum.h"
#include "material/bxdf.h"
#include "material/fresnel.h"
#include "material/microfacet_dist.h"

namespace re {

struct MicrofacetLobe {
  Spectrum f0;
  float alpha;
};

struct MicrofacetTerms {
  float3 wm;
  float cos_o;
  float cos_i;
  float d;
  float lambda_o;
  float alpha;
  bool valid;
};

RE_HD inline MicrofacetTerms microfacet_terms(const MicrofacetLobe& lobe, const float3& wo, const float3& wi) {
  MicrofacetTerms t;
  t.valid = false;

  if (!local::same_hemisphere(wo, wi)) return t;

  t.cos_o = local::abs_cos_theta(wo);
  t.cos_i = local::abs_cos_theta(wi);
  if (t.cos_o == 0.0f || t.cos_i == 0.0f) return t;

  float3 wm = wo + wi;
  const float len = length(wm);
  if (len == 0.0f) return t;
  wm = wm * (1.0f / len);
  if (wm.z < 0.0f) wm = -wm;

  t.alpha = clamp_alpha(lobe.alpha);
  t.wm = wm;
  t.d = ggx_d(wm, t.alpha);
  t.lambda_o = ggx_lambda(wo, t.alpha);
  t.valid = true;
  return t;
}

RE_HD inline Spectrum microfacet_eval_terms(const MicrofacetLobe& lobe, const float3& wo, const MicrofacetTerms& t, const float3& wi) {
  if (!t.valid) return make_empty_spectrum();
  const float lambda_i = ggx_lambda(wi, t.alpha);
  const float g2 = 1.0f / (1.0f + t.lambda_o + lambda_i);
  return fresnel_schlick(lobe.f0, fabsf(dot(wo, t.wm))) * (t.d * g2 / (4.0f * t.cos_o * t.cos_i));
}

RE_HD inline float microfacet_pdf_terms(const MicrofacetTerms& t) {
  if (!t.valid) return 0.0f;
  const float g1 = 1.0f / (1.0f + t.lambda_o);
  return g1 * t.d / (4.0f * t.cos_o);
}

RE_HD inline Spectrum microfacet_eval(const MicrofacetLobe& lobe, const float3& wo, const float3& wi) {
  return microfacet_eval_terms(lobe, wo, microfacet_terms(lobe, wo, wi), wi);
}

RE_HD inline float microfacet_pdf(const MicrofacetLobe& lobe, const float3& wo, const float3& wi) {
  return microfacet_pdf_terms(microfacet_terms(lobe, wo, wi));
}

RE_HD inline BsdfSample microfacet_sample(const MicrofacetLobe& lobe, const float3& wo, float2 u) {
  if (wo.z == 0.0f) return invalid_sample();

  const float alpha = clamp_alpha(lobe.alpha);

  const bool flip = wo.z < 0.0f;
  const float3 wo_u = flip ? -wo : wo;

  const float3 wm = ggx_sample_vndf(wo_u, alpha, u);
  const float3 wi_u = reflect(wo_u, wm);

  if (wi_u.z <= 0.0f) return invalid_sample();

  const float3 wi = flip ? -wi_u : wi_u;

  const MicrofacetTerms t = microfacet_terms(lobe, wo, wi);

  BsdfSample s;
  s.wi = wi;
  s.f = microfacet_eval_terms(lobe, wo, t, wi);
  s.pdf = microfacet_pdf_terms(t);
  s.is_delta = false;
  return s;
}

}