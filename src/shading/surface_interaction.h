#pragma once

#include <cstdint>

#include "core/platform.h"
#include "core/hit.h"
#include "core/ray.h"
#include "core/math.h"
#include "scene/scene_view.h"
#include "shading/ray_cone.h"

namespace re {

struct SurfaceInteraction {
  float3 p;
  float3 ng;
  float3 ns;
  float2 uv;
  uint32_t inst_id  = 0;
  uint32_t prim_id  = 0;
  uint32_t mat_id   = 0;
  float lod         = 0.0f;
  bool backface     = false;
};

RE_HD inline SurfaceInteraction make_surface_interaction(const SceneView& sv, const Hit& hit, const Ray& ray, const RayCone& cone = RayCone()) {
  const Instance& inst = sv.geometry.instances[hit.inst_id];
  const MeshView& mesh = sv.geometry.meshes[inst.mesh_id];
  
  SurfaceInteraction si;
  si.inst_id = hit.inst_id;
  si.prim_id = hit.prim_id;
  si.uv = interpolate_uv(mesh, hit.prim_id, hit.u, hit.v);
  si.mat_id = mesh.mat_ids[hit.prim_id];
  
  const Triangle tri = transform_triangle(inst.to_world, get_triangle(mesh, hit.prim_id));
  si.p = tri.get_point(hit.u, hit.v);
  
  const float3 cv          = cross(tri.v1 - tri.v0, tri.v2 - tri.v0);
  const float double_area  = length(cv);
  const float3 ng          = double_area > 0.0f ? cv / double_area : float3(0.0f, 1.0f, 0.0f);

  si.backface = dot(ng, ray.dir) > 0.0f;
  si.ng       = si.backface ? -ng : ng;

  if (mesh.normals) {
    const float3 n_local = interpolate_normal(mesh, hit.prim_id, hit.u, hit.v);
    const float3 ns = normalize(transform_normal(inst.to_local, n_local));
    si.ns = (dot(ns, si.ng) < 0.0f) ? -ns : ns;
  } else {
    si.ns = si.ng;
  }

  if (mesh.uvs != nullptr) {
    const uint32_t i0 = mesh.indices[3 * hit.prim_id + 0];
    const uint32_t i1 = mesh.indices[3 * hit.prim_id + 1];
    const uint32_t i2 = mesh.indices[3 * hit.prim_id + 2];

    const float delta = triangle_lod_constant(mesh.uvs[i0], mesh.uvs[i1], mesh.uvs[i2],
                                              double_area);
    si.lod = cone_lod(delta, cone, si.ng, ray.dir);
  }
  
  return si;
}

} // namespace re