#include "render/cpu/cpu_debug_renderer.h"

#include "core/frame.h"
#include "core/hit.h"
#include "core/spectrum.h"
#include "material/bsdf.h"
#include "material/material_factory.h"
#include "parallel/scheduler.h"


namespace re {
namespace {
  const float3 missColor(0.05f);
  const float3 kLightDir = normalize(float3{0.3f, 1.0f, 0.4f});
  const Spectrum kLightRadiance{3.0f, 2.9f, 2.7f};
  
  const Material kDebugMaterial = make_material(
    Spectrum{0.5f, 0.7f, 0.3f},
    0.5f,
    0.0f,
    make_empty_spectrum()
  );

  float3 geometric_normal(const Triangle& tri)
  {
    return normalize(cross(tri.v1 - tri.v0, tri.v2 - tri.v0));
  }

  Spectrum shade_lambert(const float3& normal, const Ray& ray)
  {
    float3 n = normal;

    const float3 wo = -ray.dir;
    if (dot(n, wo) < 0.0f) n = -n;

    const Frame frame = make_frame(n);
    const float3 wo_local = frame.to_local(wo);
    const float3 wi_local = frame.to_local(kLightDir);

    const Bsdf bsdf = make_bsdf(kDebugMaterial, wo_local);
    const Spectrum f = bsdf_eval(bsdf, wo_local, wi_local);

    const float cos_i = fmaxf(0.0f, dot(n, kLightDir));
    return f * kLightRadiance * cos_i;
  }
} // namespace

void CpuDebugRenderer::render(const PinholeParams &camera, Film &film) const
{
  const int width = film.width();
  const int height = film.height();
  parallel_tiles(*pool_, width, height, [&](const Tile& tile) {
    for (int y = tile.y0; y < tile.y1; ++y)
    {
      for (int x = tile.x0; x < tile.x1; ++x)
      {
        const Ray ray = generate_ray(camera, x, y, {0.5f, 0.5f});
        const Hit hit = accel_.intersect(ray);

        if (!is_hit(hit))
        {
          film.set_pixel(x, y, missColor);
          continue;
        }

        const Instance& inst = geometry_.instances[hit.instance_id];
        const MeshView  mesh = geometry_.meshes[inst.mesh_id];

        const Triangle tri =
            transform_triangle(inst.to_world, get_triangle(mesh, hit.primitive_id));

        const float3 ng = geometric_normal(tri);

        Spectrum color;
        switch (mode_)
        {
          case DebugMode::Normals:
            color = ng * 0.5f + 0.5f;
            break;
          case DebugMode::Lambert:
            color = shade_lambert(ng, ray);
            break;
        }

        film.set_pixel(x, y, color);
      }
    }
  });
}

} // namespace re