#pragma once

#include "core/frame.h"
#include "core/sampler.h"
#include "core/ray_offset.h"
#include "integrator/device_services.h"
#include "integrator/nee.h"
#include "integrator/path_state.h"
#include "integrator/russian_roulette.h"
#include "material/bsdf.h"
#include "material/material_factory.h"
#include "scene/scene_view.h"
#include "light/light_view.h"
#include "light/light_sample.h"

namespace re
{

template <Services S>
RE_HD inline Spectrum li_naive(const Ray &camera_ray,
                                const SceneView &scene,
                                const S &services,
                                Sampler &sampler,
                                int max_depth)
{
  PathState path = make_path_state(camera_ray);

  for (path.depth = 0; path.depth < max_depth; ++path.depth) {
    const Hit hit = services.intersect(path.ray);
    if (!is_hit(hit)) break;

    const Triangle tri = scene.triangle_at(hit.instance_id, hit.primitive_id);
    const float3    ng = tri.geometric_normal();
    const float3    wo = -path.ray.dir;
    const Material& mat = scene.material_at(hit.instance_id, hit.primitive_id);

    path.radiance = path.radiance + path.throughput * emitted_radiance(mat, ng, wo);

    const Frame frame = make_frame(ng);
    const float3 wo_local = frame.to_local(wo);
    const Bsdf bsdf = make_bsdf(mat, wo_local);
    const float u_lobe = sampler.next_1d();
    const float2 u = sampler.next_2d();
    const BsdfSample bs = bsdf_sample(bsdf, wo_local, u_lobe, u);
    if (!(bs.pdf > 0.0f)) break;

    path.throughput = path.throughput * bs.f * (local::abs_cos_theta(bs.wi) / bs.pdf);
    path.pdf_bsdf = bs.pdf;
    path.specular_bounce = bs.is_delta;
    const float3 wi = frame.to_world(bs.wi);
    const float3 p = tri.get_point(hit.u, hit.v);
    const float3 offset_n = dot(wi, ng) > 0.0f ? ng : -ng;
    const float3 origin = offset_ray_origin(p, offset_n);
    path.ray = make_ray(origin, wi);

    const float u_rr = sampler.next_1d();
    if (!russian_roulette(path.throughput, path.depth, u_rr)) break;
  }

  return path.radiance;
}

template <Services S>
RE_HD inline Spectrum li_nee(const Ray& camera_ray, const SceneView& scene,
                            const LightView& lights, const S& services, 
                            Sampler& sampler, int max_depth) 
{
  PathState path = make_path_state(camera_ray);
  for (path.depth = 0; path.depth < max_depth; ++path.depth) {
    const Hit hit = services.intersect(path.ray);
    if (!is_hit(hit)) break;

    const Triangle tri = scene.triangle_at(hit.instance_id, hit.primitive_id);
    const float3    ng = tri.geometric_normal();
    const float3    wo = -path.ray.dir;
    const Material& mat = scene.material_at(hit.instance_id, hit.primitive_id);
    
    if (path.specular_bounce) {
      path.radiance = path.radiance + path.throughput * emitted_radiance(mat, ng, wo);
    }

    const Frame frame = make_frame(ng);
    const float3 wo_local = frame.to_local(wo);
    const Bsdf bsdf = make_bsdf(mat, wo_local);
    const float3 pos = tri.get_point(hit.u, hit.v);
    
    const ShadowRecord rec = nee_propose(scene, lights, bsdf, frame, pos, ng, wo_local, path.throughput, sampler, false);
    nee_resolve(rec, services, path.radiance);

    const float u_lobe = sampler.next_1d();
    const float2 u = sampler.next_2d();
    const BsdfSample bs = bsdf_sample(bsdf, wo_local, u_lobe, u);
    if (!(bs.pdf > 0.0f)) break;
    
    path.throughput = path.throughput * bs.f * (local::abs_cos_theta(bs.wi) / bs.pdf);
    path.pdf_bsdf = bs.pdf;
    path.specular_bounce = bs.is_delta;
    const float3 wi = frame.to_world(bs.wi);
    const float3 offset_n = dot(wi, ng) > 0.0f ? ng : -ng;
    const float3 origin = offset_ray_origin(pos, offset_n);
    path.ray = make_ray(origin, wi);

    const float u_rr = sampler.next_1d();
    if (!russian_roulette(path.throughput, path.depth, u_rr)) break;
  }

  return path.radiance;
}

template <Services S>
RE_HD inline Spectrum li_mis(const Ray& camera_ray, const SceneView& scene,
                            const LightView& lights, const S& services, 
                            Sampler& sampler, int max_depth) 
{
  PathState path = make_path_state(camera_ray);
  float3 prev_pos = camera_ray.origin;

  for (path.depth = 0; path.depth < max_depth; ++path.depth) {
    const Hit hit = services.intersect(path.ray);
    if (!is_hit(hit)) break;

    const Triangle tri = scene.triangle_at(hit.instance_id, hit.primitive_id);
    const float3 ng = tri.geometric_normal();
    const float3 wo = -path.ray.dir;
    const Material& mat = scene.material_at(hit.instance_id, hit.primitive_id);
    const Spectrum emitted = emitted_radiance(mat, ng, wo);
    if (luminance(emitted) > 0.0f) {
      float weight = 1.0f;

      if (!path.specular_bounce) {
        const int light_index = find_light(lights, hit.instance_id, hit.primitive_id);
        const float pdf_light = light_pdf(scene, lights, light_index, prev_pos, hit.u, hit.v);
        weight = power_heuristic(path.pdf_bsdf, pdf_light);
      }

      path.radiance = path.radiance + path.throughput * emitted * weight;
    }

    const Frame frame = make_frame(ng);
    const float3 wo_local = frame.to_local(wo);
    const Bsdf bsdf = make_bsdf(mat, wo_local);

    const float3 pos = tri.get_point(hit.u, hit.v);

    const ShadowRecord rec = nee_propose(scene, lights, bsdf, frame, pos, ng, wo_local, path.throughput, sampler, true);
    nee_resolve(rec, services, path.radiance);
    
    const float u_lobe = sampler.next_1d();
    const float2 u = sampler.next_2d();
    const BsdfSample bs = bsdf_sample(bsdf, wo_local, u_lobe, u);
    if (!(bs.pdf > 0.0f)) break;

    path.throughput = path.throughput * bs.f * (local::abs_cos_theta(bs.wi) / bs.pdf);
    path.pdf_bsdf = bs.pdf;
    path.specular_bounce = bs.is_delta;
    
    const float3 wi = frame.to_world(bs.wi);
    const float3 offset_n = dot(wi, ng) > 0.0f ? ng : -ng;
    const float3 origin = offset_ray_origin(pos, offset_n);
    path.ray = make_ray(origin, wi);

    const float u_rr = sampler.next_1d();
    if (!russian_roulette(path.throughput, path.depth, u_rr)) break;
  }

  return path.radiance;
}
}
