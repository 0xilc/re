#pragma once

#include "core/frame.h"
#include "core/math.h"
#include "core/sampling.h"
#include "core/platform.h"
#include "material/bxdf.h"
#include "core/spectrum.h"

namespace re {

struct LambertianLobe {
  Spectrum albedo;
};

RE_HD inline Spectrum lambertian_eval(const LambertianLobe& lobe, const float3& wo, const float3& wi) {
  if (!local::same_hemisphere(wo, wi)) return make_empty_spectrum();
  return lobe.albedo * kInvPi;
}

RE_HD inline float lambertian_pdf(const float3& wo, const float3& wi) {
  if (!local::same_hemisphere(wo, wi)) return 0.0f;
  return pdf_cosine_hemisphere(local::abs_cos_theta(wi));
}

RE_HD inline BsdfSample lambertian_sample(const LambertianLobe& lobe, const float3& wo, float2 u) {
  if (wo.z == 0.0f) return invalid_sample();
  
  float3 wi = sample_cosine_hemisphere(u);
  if (wo.z < 0.0f) wi.z = -wi.z;
  
  BsdfSample s;
  s.wi = wi;
  s.f = lobe.albedo * kInvPi;
  s.pdf = pdf_cosine_hemisphere(local::abs_cos_theta(wi));

  return s;
}

}