#include "render/cpu/cpu_debug_renderer.h"

#include "accel/intersect_triangle.h"
#include "core/frame.h"
#include "core/hit.h"
#include "core/spectrum.h"
#include "material/bsdf.h"
#include "material/material_factory.h"

namespace re
{

namespace
{
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

  Spectrum shade_lambert(const Triangle& tri, const Ray& ray)
  {
    float3 n = geometric_normal(tri);

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
}

void CpuDebugRenderer::render(const PinholeParams &camera, Film &film) const
{
  const int width = film.width();
  const int height = film.height();

  for (int y = 0; y < height; ++y)
  {
    for (int x = 0; x < width; ++x)
    {
      const Ray ray = generate_ray(camera, x, y, {0.5f, 0.5f});
      const Hit hit = accel_.intersect(ray);

      if (!is_hit(hit))
      {
        film.set_pixel(x, y, missColor);
        continue;
      }

      const Triangle tri = get_triangle(geometry_, hit.primitive_id);

      Spectrum color;
      switch (mode_)
      {
        case DebugMode::Normals:
          color = geometric_normal(tri) * 0.5f + 0.5f;
          break;
        case DebugMode::Lambert:
          color = shade_lambert(tri, ray);
          break;
      }

      film.set_pixel(x, y, color);
    }
  }
}

}