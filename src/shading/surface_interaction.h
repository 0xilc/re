#pragma once

#include <cstdint>

#include "core/platform.h"
#include "core/hit.h"
#include "core/ray.h"
#include "core/math.h"
#include "scene/scene_view.h"
#include "core/log.h"
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

RE_HD inline SurfaceInteraction unpack(const SceneView& sv, const Hit& hit, const Ray& ray) {
  const Instance& inst = sv.geometry.instances[hit.instance_id];
  const MeshView& mesh = sv.geometry.meshes[inst.mesh_id];
  
  SurfaceInteraction si;
  si.inst_id = hit.instance_id;
  si.prim_id = hit.primitive_id;
  si.p = ray.origin + ray.dir * hit.t;
  si.uv = interpolate_uv(mesh, hit.primitive_id, hit.u, hit.v);
  si.mat_id = mesh.mat_ids[hit.primitive_id];

  const Triangle tri = transform_triangle(inst.to_world, get_triangle(mesh, hit.primitive_id));
  float3 ng = tri.geometric_normal();

  si.backface = dot(ng, ray.dir) > 0.0f;
  si.ng = si.backface ? -ng : ng;

  if (mesh.normals) {
  const float3 n_local = interpolate_normal(mesh, hit.primitive_id, hit.u, hit.v);
  const float3 ns = normalize(transform_normal(inst.to_local, n_local));
  si.ns = (dot(ns, si.ng) < 0.0f) ? -ns : ns;
  } else {
    si.ns = si.ng;
  }
  
  return si;
}

} // namespace re