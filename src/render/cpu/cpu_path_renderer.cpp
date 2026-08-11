#include "render/cpu/cpu_path_renderer.h"

#include "core/sampler.h"
#include "integrator/path_integrator.h"

namespace re {

void CpuPathRenderer::render(const PinholeParams& camera, Film& film) const {
  const int width = film.width();
  const int height = film.height();
  const int spp = settings_.samples_per_pixel;
  const float inv_spp = 1.0f / static_cast<float>(spp);

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      Spectrum sum = make_empty_spectrum();

      for (int s = 0; s < spp; ++s) {
        Sampler sampler(x, y, s);

        const Ray ray = generate_ray(camera, x, y, sampler.next_2d());

        sum = sum + li_native(ray, scene_, services_, sampler, settings_.max_depth);
      }

      film.set_pixel(x, y, sum * inv_spp);
    }
  }
}

}