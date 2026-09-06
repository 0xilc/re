#pragma once

#include "core/frame.h"
#include "core/sampler.h"
#include "core/ray_offset.h"
#include "integrator/device_services.h"
#include "integrator/nee.h"
#include "integrator/path_state.h"
#include "integrator/russian_roulette.h"
#include "material/bsdf.h"
#include "material/bsdf_build.h"
#include "scene/scene_view.h"
#include "light/area_light.h"
#include "shading/evaluated_material.h"
#include "shading/normal_map.h"
#include "shading/surface_interaction.h"

namespace re {

template <Services S>
RE_HD inline Spectrum li_naive(const Ray &camera_ray,
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

    const float3 ng = si.backface ? -si.ng : si.ng;
    const float3 wo = -path.ray.dir;
    const EvaluatedMaterial mat =
        evaluate_material(scene.materials[si.mat_id], si, services);

    path.radiance = path.radiance + path.throughput * emitted_radiance(mat, ng, wo);

    const Frame frame = make_frame(si.ns);
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
    if (dot(wi, ng) <= 0.0f) break;
    
    const float3 origin = offset_ray_origin(si.p, ng);
    path.ray = make_ray(origin, wi);

    const float u_rr = sampler.next_1d();
    if (!russian_roulette(path.throughput, path.depth, u_rr)) break;
  }

  return path.radiance;
}

template <Services S, LightSelector Lights>
RE_HD inline Spectrum li_nee(const Ray& camera_ray, const RayCone& camera_cone,
                            const SceneView& scene,
                            const Lights& lights, const S& services,
                            Sampler& sampler, int max_depth)
{
  PathState path = make_path_state(camera_ray, camera_cone);
  for (path.depth = 0; path.depth < max_depth; ++path.depth) {
    const Hit hit = services.intersect(path.ray);
    if (!is_hit(hit)) break;

    path.cone = propagate(path.cone, hit.t);

    const SurfaceInteraction si = make_surface_interaction(scene, hit, path.ray, path.cone);

    const float3 ng = si.backface ? -si.ng : si.ng;
    const float3 wo = -path.ray.dir;
    const EvaluatedMaterial mat =
        evaluate_material(scene.materials[si.mat_id], si, services);

    if (path.specular_bounce) {
      path.radiance = path.radiance + path.throughput * emitted_radiance(mat, ng, wo);
    }

    const Frame frame = make_frame(si.ns);
    const float3 wo_local = frame.to_local(wo);
    const Bsdf bsdf = make_bsdf(mat, wo_local);
    const float3 pos = si.p;
    
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
    if (dot(wi, ng) <= 0.0f) break;

    const float3 origin = offset_ray_origin(pos, ng);
    path.ray = make_ray(origin, wi);

    const float u_rr = sampler.next_1d();
    if (!russian_roulette(path.throughput, path.depth, u_rr)) break;
  }

  return path.radiance;
}

template <Services S, LightSelector Lights>
RE_HD inline Spectrum li_mis(const Ray& camera_ray, const RayCone& camera_cone, const SceneView& scene,
                            const Lights& lights, const S& services, 
                            Sampler& sampler, int max_depth) 
{
  PathState path = make_path_state(camera_ray, camera_cone);
  LightSampleContext prev_ctx{camera_ray.origin, -camera_ray.dir};

  for (path.depth = 0; path.depth < max_depth; ++path.depth) {
    const Hit hit = services.intersect(path.ray);
    if (!is_hit(hit)) break;
    path.cone = propagate(path.cone, hit.t);

    const SurfaceInteraction si = make_surface_interaction(scene, hit, path.ray, path.cone);

    const float3 ng = si.backface ? -si.ng : si.ng;
    const float3 wo = -path.ray.dir;

    const Material&         material = scene.materials[si.mat_id];
    const EvaluatedMaterial mat      = evaluate_material(material, si, services);

    const Spectrum emitted = emitted_radiance(mat, ng, wo);
    if (luminance(emitted) > 0.0f) {
      float weight = 1.0f;

      if (!path.specular_bounce) {
        const LightRef ref{hit.inst_id, hit.prim_id};
        const float pdf_light = area_light_pdf(scene, lights, prev_ctx, ref, hit.u, hit.v);
        weight = power_heuristic(path.pdf_bsdf, pdf_light);
      }

      path.radiance = path.radiance + path.throughput * emitted * weight;
    }

    float3 ns = si.ns;

    if (material.normal_tex != kNoTexture) {
      const Instance& inst = scene.geometry.instances[si.inst_id];
      const MeshView& mesh = scene.geometry.meshes[inst.mesh_id];

      if (mesh.tangents != nullptr) {
        const float4 tangent =
            interpolate_tangent(mesh, si.prim_id, hit.u, hit.v);

        float3 t = transform_vector(inst.to_world, tangent.xyz);
        t = t - si.ns * dot(si.ns, t);

        if (length_squared(t) > 1e-12f) {
          const Frame  tangent_frame = make_frame(si.ns, t, tangent.w);
          const float4 texel = services.sample_texture(material.normal_tex, si.uv, si.lod);

          ns = apply_normal_map(texel, material.normal_scale, tangent_frame);
        }
      }
    }

    const Frame frame = make_frame(ns);
    const float3 wo_local = frame.to_local(wo);
    const Bsdf bsdf = make_bsdf(mat, wo_local);

    const float3 pos = si.p;

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
    if (dot(wi, ng) <= 0.0f) break;

    const float3 origin = offset_ray_origin(pos, ng);
    path.ray = make_ray(origin, wi);
    prev_ctx = LightSampleContext{pos, ng};

    const float u_rr = sampler.next_1d();
    if (!russian_roulette(path.throughput, path.depth, u_rr)) break;
  }

  return path.radiance;
}
} // namespace re
