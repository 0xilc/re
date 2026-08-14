#include "render/cpu/cpu_path_renderer.h"

#include "core/sampler.h"
#include "integrator/path_integrator.h"
#include "parallel/scheduler.h"

namespace re {

void CpuPathRenderer::render(const PinholeParams& camera, Film& film) const {
  const int width = film.width();
  const int height = film.height();
  const int spp = settings_.samples_per_pixel;
  const float inv_spp = 1.0f / static_cast<float>(spp);

  parallel_tiles(*pool_, width, height, [&](const Tile& tile) {
    for (int y = tile.y0; y < tile.y1; ++y) {
      for (int x = tile.x0; x < tile.x1; ++x) {
        Spectrum sum = make_empty_spectrum();

        for (int s = 0; s < spp; ++s) {
          Sampler sampler(x, y, s);
          const Ray ray = generate_ray(camera, x, y, sampler.next_2d());

          switch (settings_.mode) {
            case IntegratorMode::Naive:
              sum = sum + li_naive(ray, scene_, services_, sampler, settings_.max_depth);
              break;
            case IntegratorMode::Nee:
              sum = sum + li_nee(ray, scene_, lights_, services_, sampler, settings_.max_depth);
              break;
            case IntegratorMode::Mis:
              sum = sum + li_mis(ray, scene_, lights_, services_, sampler, settings_.max_depth);
              break;
          }
        }

        film.set_pixel(x, y, sum * inv_spp);
      }
    }
  });
}

}