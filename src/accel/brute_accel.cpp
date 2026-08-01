#include "accel/brute_accel.h"
#include "accel/intersect_triangle.h"

namespace re {

void BruteAccel::build(const GeometryView& mesh) {
  mesh_ = mesh;
}

Hit BruteAccel::intersect(const Ray& ray) const {
  Hit best{};
  best.t = ray.tmax;

  for (int primitive_id = 0; primitive_id < mesh_.triangle_count; ++primitive_id) {
    TriangleHit th;
    if (!intersect_triangle(ray, get_triangle(mesh_, primitive_id), th)) continue;
    if (th.t >= best.t) continue; 

    best.t = th.t;
    best.u = th.u;
    best.v = th.v;
    best.primitive_id = primitive_id;
    best.instance_id = 0;
  }

  return best;
}

}