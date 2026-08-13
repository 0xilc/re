#pragma once

#include "core/math.h"
#include "core/sampling.h"
#include "core/spectrum.h"
#include "light/light_view.h"
#include "scene/material.h"
#include "scene/scene_view.h"

namespace re {

struct LightSample {
  float3    wi;         // In world space
  float     dist;       // Distance to the sampled point
  Spectrum  radiance;   // L arriving from that direction
  float     pdf;        // Solid angle, pick probability counted
  bool      is_delta;   // no MIS weight for this
};

RE_HD inline LightSample invalid_light_sample() {
  return LightSample {
    .wi = float3{0.0f, 0.0f, 1.0f},
    .dist = 0.0f,
    .radiance = Spectrum {0.0f, 0.0f, 0.0f},
    .pdf = 0.0f,
    .is_delta = false
  };
}

// Jacobian Transformation
RE_HD inline float area_to_solid_angle_pdf(float pdf_area, float dist_sq, float cos_light) {
  if (cos_light <= 0.0f) return 0.0f;
  return pdf_area * dist_sq / cos_light;
}

RE_HD inline LightSample sample_light(const SceneView& scene, const LightView& lights,
                                      const float3& shade_pos, float u_pick, float2 u_pos) 
{
  const LightPick pick = pick_light(lights, u_pick);
  if (pick.index < 0 || !(pick.pdf > 0.0f)) return invalid_light_sample();

  const int prim = lights.prims[pick.index];
  const Triangle tri = get_triangle(scene.geometry, prim);
  const float2 bary = sample_uniform_triangle(u_pos);
  const float3 p = tri.get_point(bary.x, bary.y);
  const float3 n = tri.geometric_normal();

  const float3 to_light = p - shade_pos;
  const float dist_sq = dot(to_light, to_light);
  if (dist_sq <= 0.0f) return invalid_light_sample();
  
  const float dist =  sqrtf(dist_sq);
  const float3 wi = to_light * (1.0f / dist);
  const Material& mat = scene.material_at(prim);
  const Spectrum L = emitted_radiance(mat, n, -wi);

  const float cos_light = dot(n, -wi);
  const float area = tri.area();
  if (area <= 0.0f) return invalid_light_sample();

  const float pdf = area_to_solid_angle_pdf(pick.pdf / area, dist_sq, cos_light);
  if (!(pdf > 0.0f)) return invalid_light_sample();

  return LightSample{wi, dist, L, pdf, false};
}

RE_HD inline float light_pdf(const SceneView& scene, const LightView& lights, 
                             int light_index, const float3& shade_pos, 
                             float u_u, float u_v) 
{
  if (light_index < 0) return 0.0f;

  const int prim = lights.prims[light_index];
  const Triangle tri = get_triangle(scene.geometry, prim);
  const float area = tri.area();
  if (area <= 0.0f) return 0.0f;

  const float3 p = tri.get_point(u_u, u_v);
  const float3 n = tri.geometric_normal();

  const float3 to_light = p - shade_pos;
  const float dist_sq = dot(to_light, to_light);
  if (dist_sq <= 0.0f) return 0.0f;

  const float3 wi = to_light * (1.0f / sqrtf(dist_sq));
  const float cos_light = dot(n, -wi);
  
  return area_to_solid_angle_pdf(pick_light_pdf(lights, light_index) / area, dist_sq, cos_light);
}

}