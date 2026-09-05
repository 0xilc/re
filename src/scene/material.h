#pragma once

#include <cstdint>

#include "core/platform.h"
#include "core/spectrum.h"
#include "core/types.h"
#include "core/math.h"

namespace re {

inline constexpr uint32_t kNoTexture = 0xFFFFFFFFu;

struct EvaluatedMaterial {
  Spectrum base_color;
  Spectrum emission;
  float    metallic;
  float    roughness;
  float    ior;

  RE_HD bool is_emissive() const {
    return emission.x > 0.0f || emission.y > 0.0f || emission.z > 0.0f;
  }
};

struct alignas(64) Material {
  Spectrum  base_color;
  Spectrum  emission;
  float     metallic;
  float     roughness;
  float     ior;
  float     normal_scale;
  uint32_t  base_color_tex;
  uint32_t  metallic_roughness_tex;
  uint32_t  normal_tex;
  uint32_t  emissive_tex;
  int8_t    priority;
  uint8_t   reserved[7];

  RE_HD inline bool is_emissive() const {
    return emission.x > 0.0f || emission.y > 0.0f || emission.z > 0.0f; 
  }
  
  RE_HD inline EvaluatedMaterial evaluated() const {
    return EvaluatedMaterial{base_color, emission, metallic, roughness, ior};
  }
};

RE_HD inline Material make_material(const Spectrum& base_color, 
                                    float metallic, 
                                    float roughness, 
                                    const Spectrum& emission = Spectrum{0.0f, 0.0f, 0.0f}) {
  Material m;
  m.base_color              = base_color;
  m.emission                = emission;
  m.metallic                = metallic;
  m.roughness               = roughness;
  m.ior                     = 1.5f;
  m.normal_scale            = 1.0f;
  m.base_color_tex          = kNoTexture;
  m.metallic_roughness_tex  = kNoTexture;
  m.normal_tex              = kNoTexture;
  m.emissive_tex            = kNoTexture;
  m.priority                = 0;

  return m;
}

RE_HD inline Spectrum emitted_radiance(const EvaluatedMaterial& m, 
  const float3& n_light, const float3& w) {
    return dot(n_light, w) > 0.0f ? m.emission : make_empty_spectrum();
}

}

