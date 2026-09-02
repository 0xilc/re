#pragma once

#include <cstdint>
#include <concepts>
#include "core/hit.h"
#include "core/ray.h"

namespace re {

template <typename S>
concept Services = requires(const S& services, const Ray& ray, uint32_t tex_id, float2 uv, float lod) {
  
  // Closest hit, returns INVALID_ID on miss
  { services.intersect(ray) } -> std::same_as<Hit>;

  // Any hit
  { services.occluded(ray) }  -> std::same_as<bool>;
  
  { services.sample_texture(tex_id, uv, lod) } -> std::same_as<float4>;
};

}