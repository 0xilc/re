#pragma once

#include "core/frame.h"
#include "core/ray_offset.h"
#include "core/sampler.h"
#include "integrator/device_services.h"
#include "light/light_sample.h"
#include "material/bsdf.h"


namespace re {

namespace {
constexpr float kShadowRayScale = 0.9999f;
} // namespace

struct ShadowRecord {
  Ray       ray;
  Spectrum  contribution;
  bool      valid;
};

RE_HD inline ShadowRecord invalid_shadow_record() {
  ShadowRecord r;
  r.contribution = Spectrum{0.0f, 0.0f, 0.0f};
  r.valid = false;
  return r;
}

RE_HD inline ShadowRecord nee_propose(const SceneView& scene, const LightView& lights,
                                      const Bsdf& bsdf, const Frame& frame, const float3& pos,
                                      const float3& ng, const float3& wo_local,
                                      const Spectrum& throughput, Sampler& sampler) 
{
  const float u_pick = sampler.next_1d();
  const float2 u_pos = sampler.next_2d();
  
  if (lights.count <= 0) return invalid_shadow_record();

  const LightSample ls = sample_light(scene, lights, pos, u_pick, u_pos);
  if (!(ls.pdf > 0.0f)) return invalid_shadow_record();

  const float3 wi_local = frame.to_local(ls.wi);
  const Spectrum f = bsdf_eval(bsdf, wo_local, wi_local);
  const float cos_i = local::abs_cos_theta(wi_local);
  if (cos_i <= 0.0f) return invalid_shadow_record();

  const Spectrum contribution = throughput * f * ls.radiance * (cos_i / ls.pdf);
  if (luminance(contribution) <= 0.0f) return invalid_shadow_record();
  const float3 offset_n = dot(ls.wi, ng) > 0.0f ? ng : -ng;

  ShadowRecord rec;
  rec.ray = make_ray(offset_ray_origin(pos, offset_n), ls.wi);
  rec.ray.tmax = ls.dist * kShadowRayScale;
  rec.contribution = contribution;
  rec.valid = true;

  return rec;
}

template <Services S>
RE_HD inline void nee_resolve(const ShadowRecord& rec, const S& services, Spectrum& radiance) {
  if (!rec.valid) return;
  if (services.occluded(rec.ray)) return;
  radiance = radiance + rec.contribution;
}

}