#pragma once

#include "material/bxdf.h"
#include "material/bxdf_lambertian.h"
#include "material/bxdf_microfacet.h"
#include <cstdint>

namespace re {

constexpr inline int kMaxLobes = 4;

enum struct LobeTag : uint8_t {
  Lambertian,
  Microfacet,
};

struct Lobe {
  LobeTag tag;
  union {
    LambertianLobe lambertian;
    MicrofacetLobe microfacet;
  };
};

RE_HD inline Spectrum lobe_eval(const Lobe& lobe, const float3& wo, const float3& wi) {
  switch (lobe.tag) {
    case LobeTag::Lambertian:
      return lambertian_eval(lobe.lambertian, wo, wi);
    case LobeTag::Microfacet:
      return microfacet_eval(lobe.microfacet, wo, wi);
  }

  return make_empty_spectrum();
}

RE_HD inline float lobe_pdf(const Lobe& lobe, const float3& wo, const float3& wi) {
  switch (lobe.tag) {
    case LobeTag::Lambertian:
      return lambertian_pdf(wo, wi);
    case LobeTag::Microfacet:
      return microfacet_pdf(lobe.microfacet, wo, wi);
  }

  return 0.0f;
}

RE_HD inline BsdfSample lobe_sample(const Lobe& lobe, const float3& wo, float2 u) {
  switch (lobe.tag) {
    case LobeTag::Lambertian:
      return lambertian_sample(lobe.lambertian, wo, u);
    case LobeTag::Microfacet:
      return microfacet_sample(lobe.microfacet, wo, u);      
  }

  return invalid_sample();
}

// --- BSDF
struct Bsdf {
  Lobe lobes[kMaxLobes];
  float selected_prob[kMaxLobes];
  int count = 0;
};

RE_HD inline Spectrum bsdf_eval(const Bsdf& bsdf, const float3& wo, const float3& wi) {
  Spectrum f = make_empty_spectrum();
  for (int i = 0; i < bsdf.count; ++i) {
    f = f + lobe_eval(bsdf.lobes[i], wo, wi);
  }

  return f;
}

RE_HD inline float bsdf_pdf(const Bsdf& bsdf, const float3& wo, const float3& wi) {
  float pdf = 0.0f;
  for (int i = 0; i < bsdf.count; ++i) {
    pdf += bsdf.selected_prob[i] * lobe_pdf(bsdf.lobes[i], wo, wi);
  }

  return pdf;
}

RE_HD inline BsdfSample bsdf_sample(const Bsdf& bsdf, const float3& wo, float u_lobe, float2 u) {
  if (bsdf.count == 0) return invalid_sample();
  
  // Sample lobe
  int index = bsdf.count - 1;
  float acc = 0.0f;
  for (int i = 0; i < bsdf.count - 1; ++i) {
    acc += bsdf.selected_prob[i];
    if (u_lobe < acc) {
      index = i;
      break;
    }
  }

  const BsdfSample picked = lobe_sample(bsdf.lobes[index], wo, u);
  if (!(picked.pdf > 0.0f)) return invalid_sample();

  if (picked.is_delta) {
    BsdfSample s = picked;
    s.pdf = bsdf.selected_prob[index] * picked.pdf;
    return s;
  }

  BsdfSample s;
  s.wi = picked.wi;
  s.f = bsdf_eval(bsdf, wo, picked.wi);
  s.pdf = bsdf_pdf(bsdf, wo, picked.wi);
  s.is_delta = false;

  return s.pdf > 0.0f ? s : invalid_sample();
}

RE_HD inline void bsdf_add_lobe(Bsdf& bsdf, const Lobe& lobe, float weight) {
  if (bsdf.count >= kMaxLobes || !(weight > 0.0f)) return;
  bsdf.lobes[bsdf.count] = lobe;
  bsdf.selected_prob[bsdf.count] = weight;
  ++bsdf.count;
}

RE_HD inline void bsdf_finalize(Bsdf& bsdf) {
  float sum = 0.0f;
  for (int i = 0; i < bsdf.count; ++i) {
    sum += bsdf.selected_prob[i];
  }
  if (!(sum > 0.0f)) {
    bsdf.count = 0;
    return;
  }

  const float inv = 1.0f / sum;
  for (int i = 0; i < bsdf.count; ++i) {
    bsdf.selected_prob[i] *= inv;
  }
}
}