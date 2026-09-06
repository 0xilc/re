#include "accel/brute_accel.h"
#include "accel/intersect_triangle.h"

namespace re {

void BruteAccel::build(const SceneGeometryView& scene_geometry) {
  scene_geometry_ = scene_geometry;
}

Hit BruteAccel::intersect(const Ray& ray) const {
  const Instance* instances     = scene_geometry_.instances;
  const MeshView* meshes        = scene_geometry_.meshes;
  const uint32_t instance_count = scene_geometry_.instance_count;

  Hit best{};
  best.t = ray.tmax;
  
  for (uint32_t inst_id = 0; inst_id < instance_count; ++inst_id) {
    const Instance& inst  = instances[inst_id];
    const MeshView mesh   = meshes[inst.mesh_id];
    const Ray local       = transform_ray(inst.to_local, ray);

    for (uint32_t prim_id = 0; prim_id < mesh.triangle_count; ++prim_id) {
      TriangleHit th;
      if (!intersect_triangle(local, get_triangle(mesh, prim_id), th)) continue;
      if (th.t >= best.t) continue;

      best.t = th.t;
      best.u = th.u;
      best.v = th.v;
      best.prim_id = prim_id;
      best.inst_id = inst_id;
    }
  }

  return best;
}

bool BruteAccel::occluded(const Ray& ray) const {
  const Instance* instances     = scene_geometry_.instances;
  const MeshView* meshes        = scene_geometry_.meshes;
  const uint32_t instance_count = scene_geometry_.instance_count;
  
  for (uint32_t inst_id = 0; inst_id < instance_count; ++inst_id) {
    const Instance& inst  = instances[inst_id];
    const MeshView mesh   = meshes[inst.mesh_id];
    const Ray local       = transform_ray(inst.to_local, ray);

    for (uint32_t prim_id = 0; prim_id < mesh.triangle_count; ++prim_id) {
      TriangleHit th;
      if (intersect_triangle(local, get_triangle(mesh, prim_id), th)) {
        return true;
      }
    }
  }
  
  return false;
}

}