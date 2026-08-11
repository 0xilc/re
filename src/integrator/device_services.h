#pragma once

#include <concepts>
#include "core/hit.h"
#include "core/ray.h"

namespace re {

template <typename S>
concept Services = requires(const S& services, const Ray& ray) {
  
  // Closest hit, returns INVALID_ID on miss
  { services.intersect(ray) } -> std::same_as<Hit>;

  // Any hit
  { services.occluded(ray) }  -> std::same_as<bool>;

};

}