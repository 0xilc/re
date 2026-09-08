#include "render/cpu/cpu_debug_renderer.h"

#include "core/frame.h"
#include "core/hit.h"
#include "core/spectrum.h"
#include "material/bsdf.h"
#include "material/bsdf_build.h"
#include "parallel/scheduler.h"
#include "shading/ray_cone.h"
#include "shading/surface_interaction.h"


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
        const Hit hit = services_.intersect(ray);

        if (!is_hit(hit))
        {
          film.set_pixel(x, y, missColor);
          continue;
        }
        const RayCone cone = RayCone{
          .width = 0.0f,
          .spread = camera.pixel_spread_angle
        };
        const SurfaceInteraction si = make_surface_interaction(scene_, hit, ray, propagate(cone, hit.t));

        Spectrum color;
        switch (mode_)
        {
          case DebugMode::UV: {
            color = Spectrum{si.uv.x, si.uv.y, 0.0f};
            break;
          }
          case DebugMode::Normals: {
            color = si.ns * 0.5f + 0.5f;
            break;
          }
          case DebugMode::Lambert: {
            color = shade_lambert(si.ng, ray);
            break;
          }
          case DebugMode::BaseColor: {
            const Material& mat = scene_.materials[si.mat_id];
            const float4 texel = services_.sample_texture(mat.base_color_tex, si.uv, si.lod);
            color = mat.base_color * texel.xyz;
            break;
          }
          case DebugMode::Lod: {
            const RayCone at_hit = propagate(cone, hit.t);
            const float t = clamp(at_hit.width / (cone.spread * 5.0f), 0.0f, 1.0f);
            color = Spectrum{t, 1.0f - t, 0.0f};
            break;
          }
        }

        film.set_pixel(x, y, color);
      }
    }
  });
}

} // namespace re