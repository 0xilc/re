#pragma once

#include "core/frame.h"
#include "core/sampler.h"
#include "core/ray_offset.h"
#include "integrator/device_services.h"
#include "integrator/path_state.h"
#include "material/bsdf.h"
#include "material/material_factory.h"
#include "scene/scene_view.h"

namespace re
{

template <Services S>
RE_HD inline Spectrum li_native(const Ray &camera_ray,
                                const SceneView &scene,
                                const S &services,
                                Sampler &sampler,
                                int max_depth)
{
  PathState path = make_path_state(camera_ray);

  for (path.depth = 0; path.depth < max_depth; ++path.depth) {
    const Hit hit = services.intersect(path.ray);
    if (!is_hit(hit)) break;

    const Triangle tri = get_triangle(scene.geometry, hit.primitive_id);
    const float3    ng = tri.geometric_normal();
    const float3    wo = -path.ray.dir;
    const Material& mat = scene.material_at(hit.primitive_id);

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
    path.ray = make_ray(offset_ray_origin(p, offset_n), wi);
  }

  return path.radiance;
}

}
