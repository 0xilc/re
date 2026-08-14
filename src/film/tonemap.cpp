#include "film/tonemap.h"
#include "core/math.h"
#include <cmath>
#include <cstdint>

namespace re {

float3 tonemap_reinhard(const float3& c) {
  return float3{
    c.x / (1.0f + c.x),
    c.y / (1.0f + c.y),
    c.z / (1.0f + c.z)
  };
}

float3 tonemap(const float3& c, Tonemap mapper) {
  switch (mapper) {
    case Tonemap::None:
      return c;
    case Tonemap::Reinhard:
      return tonemap_reinhard(c);
  }
  return c;
}

float linear_to_srgb(float linear) {
  linear = clamp(linear, 0.0f, 1.0f);
  return linear <= 0.0031308f ? 12.92f * linear
                : 1.055f * std::pow(linear, 1.0f / 2.4f) - 0.055f;
}

std::vector<uint8_t> encode_srgb8(const Film& film) {
  const std::vector<float3>& src = film.pixels();

  std::vector<uint8_t> out(src.size() * 3);

  for (size_t i = 0; i < src.size(); ++i) {
    const float3& c = src[i];

    out[i * 3 + 0] = static_cast<uint8_t>(linear_to_srgb(c.x) * 255.0f + 0.5f);
    out[i * 3 + 1] = static_cast<uint8_t>(linear_to_srgb(c.y) * 255.0f + 0.5f);
    out[i * 3 + 2] = static_cast<uint8_t>(linear_to_srgb(c.z) * 255.0f + 0.5f);
  }

  return out;
}

}