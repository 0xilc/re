#pragma once

#include "core/frame.h"
#include "core/sampler.h"
#include "core/ray_offset.h"
#include "core/spectrum.h"
#include "integrator/device_services.h"
#include "integrator/nee.h"
#include "integrator/path_state.h"
#include "integrator/russian_roulette.h"
#include "material/bsdf.h"
#include "material/bsdf_build.h"
#include "scene/scene_view.h"
#include "light/area_light.h"
#include "shading/evaluated_material.h"
#include "shading/shading_context.h"
#include "shading/surface_interaction.h"

namespace re {

template <Services S>
RE_HD inline Spectrum li_naive( const Ray &camera_ray,
                                const RayCone &camera_cone,
                                const SceneView &scene,
                                const S &services,
                                Sampler &sampler,
                                int max_depth)
{
  PathState path = make_path_state(camera_ray, camera_cone);

  for (path.depth = 0; path.depth < max_depth; ++path.depth) {
    const Hit hit = services.intersect(path.ray);
    if (!is_hit(hit)) break;

    path.cone = propagate(path.cone, hit.t);

    const SurfaceInteraction si = make_surface_interaction(scene, hit, path.ray, path.cone);

    const Material mat = evaluate_material(scene.materials[si.mat_id], si, services);

    path.radiance = path.radiance + path.throughput * emitted_radiance(mat, si.backface);

    const ShadingContext shading = make_shading_context(services, si, mat);

    const float u_lobe = sampler.next_1d();
    const float2 u = sampler.next_2d();
    const BsdfSample bs = bsdf_sample(shading.bsdf, shading.wo_local, u_lobe, u);
    if (!(bs.pdf > 0.0f)) break;

    path.throughput = path.throughput * bs.f * (local::abs_cos_theta(bs.wi) / bs.pdf);
    path.pdf_bsdf = bs.pdf;
    path.specular_bounce = bs.is_delta;
    const float3 wi = shading.frame.to_world(bs.wi);
    if (dot(wi, si.ng) <= 0.0f) break;
    
    const float3 origin = offset_ray_origin(si.p, si.ng);
    path.ray = make_ray(origin, wi);

    const float u_rr = sampler.next_1d();
    if (!russian_roulette(path.throughput, path.depth, u_rr)) break;
  }

  return path.radiance;
}

template <Services S, LightSelector Lights>
RE_HD inline Spectrum li_nee( const Ray& camera_ray, 
                              const RayCone& camera_cone,
                              const SceneView& scene,
                              const Lights& lights, 
                              const S& services,
                              Sampler& sampler, 
                              int max_depth)
{
  PathState path = make_path_state(camera_ray, camera_cone);
  for (path.depth = 0; path.depth < max_depth; ++path.depth) {
    const Hit hit = services.intersect(path.ray);
    if (!is_hit(hit)) break;

    path.cone = propagate(path.cone, hit.t);

    const SurfaceInteraction si = make_surface_interaction(scene, hit, path.ray, path.cone);
    const Material mat = evaluate_material(scene.materials[si.mat_id], si, services);

    if (path.specular_bounce) {
      path.radiance = path.radiance + path.throughput * emitted_radiance(mat, si.backface);
    }

    const ShadingContext shading = make_shading_context(services, si, mat);
    
    const ShadowRecord rec = nee_propose(scene, lights, shading.bsdf, shading.frame, si.p, si.ng, shading.wo_local, path.throughput, sampler, false);
    nee_resolve(rec, services, path.radiance);

    const float u_lobe = sampler.next_1d();
    const float2 u = sampler.next_2d();
    const BsdfSample bs = bsdf_sample(shading.bsdf, shading.wo_local, u_lobe, u);
    if (!(bs.pdf > 0.0f)) break;
    
    path.throughput = path.throughput * bs.f * (local::abs_cos_theta(bs.wi) / bs.pdf);
    path.pdf_bsdf = bs.pdf;
    path.specular_bounce = bs.is_delta;
    const float3 wi = shading.frame.to_world(bs.wi);
    if (dot(wi, si.ng) <= 0.0f) break;

    const float3 origin = offset_ray_origin(si.p, si.ng);
    path.ray = make_ray(origin, wi);

    const float u_rr = sampler.next_1d();
    if (!russian_roulette(path.throughput, path.depth, u_rr)) break;
  }

  return path.radiance;
}

template <Services S, LightSelector Lights>
RE_HD inline Spectrum li_mis( const Ray& camera_ray, 
                              const RayCone& camera_cone, 
                              const SceneView& scene,
                              const Lights& lights, 
                              const S& services, 
                              Sampler& sampler, 
                              int max_depth) 
{
  PathState path = make_path_state(camera_ray, camera_cone);
  LightSampleContext prev_ctx{camera_ray.origin, -camera_ray.dir};

  for (path.depth = 0; path.depth < max_depth; ++path.depth) {
    const Hit hit = services.intersect(path.ray);
    if (!is_hit(hit)) break;
    
    path.cone = propagate(path.cone, hit.t);

    const SurfaceInteraction si = make_surface_interaction(scene, hit, path.ray, path.cone);

    const Material mat = evaluate_material(scene.materials[si.mat_id], si, services);

    const Spectrum emitted = emitted_radiance(mat, si.backface);
    if (luminance(emitted) > 0.0f) {
      float weight = 1.0f;

      if (!path.specular_bounce) {
        const LightRef ref{si.inst_id, si.prim_id};
        const float pdf_light = area_light_pdf(scene, lights, prev_ctx, ref, si.bary.x, si.bary.y);
        weight = power_heuristic(path.pdf_bsdf, pdf_light);
      }

      path.radiance = path.radiance + path.throughput * emitted * weight;
    }

    const ShadingContext shading = make_shading_context(services, si, mat);

    const ShadowRecord rec = nee_propose(scene, lights, shading.bsdf, shading.frame, si.p, si.ng, shading.wo_local, path.throughput, sampler, true);
    nee_resolve(rec, services, path.radiance);
    
    const float u_lobe = sampler.next_1d();
    const float2 u = sampler.next_2d();
    const BsdfSample bs = bsdf_sample(shading.bsdf, shading.wo_local, u_lobe, u);
    if (!(bs.pdf > 0.0f)) break;

    path.throughput = path.throughput * bs.f * (local::abs_cos_theta(bs.wi) / bs.pdf);
    path.pdf_bsdf = bs.pdf;
    path.specular_bounce = bs.is_delta;
    
    const float3 wi = shading.frame.to_world(bs.wi);
    if (dot(wi, si.ng) <= 0.0f) break;

    const float3 origin = offset_ray_origin(si.p, si.ng);
    path.ray = make_ray(origin, wi);
    prev_ctx = LightSampleContext{si.p, si.ng};

    const float u_rr = sampler.next_1d();
    if (!russian_roulette(path.throughput, path.depth, u_rr)) break;
  }

  return path.radiance;
}
} // namespace re
