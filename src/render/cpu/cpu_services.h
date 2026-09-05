#pragma once

#include <cmath>

#include "core/types.h"
#include "accel/accel.h"
#include "integrator/device_services.h"
#include "render/cpu/texture_cache.h"
#include "render/cpu/cpu_texture_sample.h"

namespace re {

class CpuServices {
public:
  explicit CpuServices(const Accel& accel, const TextureCache& textures) 
    : accel_(accel), textures_(textures) {}

  Hit intersect(const Ray& ray) const { 
    return accel_.intersect(ray);
  }

  bool occluded(const Ray& ray) const { 
    return accel_.occluded(ray);
  }

  float4 sample_texture(uint32_t tex_id, float2 uv, float lod) const {
    if (tex_id >= textures_.texture_count()) return float4{1.0f, 1.0f, 1.0f, 1.0f};
    
    const CpuTexture& tex = textures_.textures()[tex_id];
    return re::sample_texture(textures_.texels(), tex, uv, texture_mip_level(tex, lod));
  }

private:
  const Accel& accel_;
  const TextureCache& textures_;
};

static_assert(Services<CpuServices>, "CpuServices must satisfy the Services concept.");
}