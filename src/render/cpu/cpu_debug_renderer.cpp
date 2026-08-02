#include "render/cpu/cpu_debug_renderer.h"

#include "accel/intersect_triangle.h"
#include "core/hit.h"

namespace re
{

namespace
{

  const float3 missColor(0.05f);

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
      const float3 n = normalize(cross(tri.v1 - tri.v0, tri.v2 - tri.v0));

      film.set_pixel(x, y, n * 0.5f + 0.5f);
    }
  }
}

}