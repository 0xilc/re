#pragma once 

#include <cassert>
#include <cmath>
#include <cstdint>

#include "core/color.h"
#include "core/types.h"
#include "core/math.h"
#include "texture/texture_build.h"
#include "render/cpu/texture_cache.h"

namespace re {

inline uint32_t wrap_index(int i, uint32_t n, WrapMode mode) {
  const int size = static_cast<int>(n);

  if (mode == WrapMode::Clamp) {
    return static_cast<uint32_t>(i < 0 ? 0 : (i >= size ? size - 1 : i));
  }

  const int m = i % size;
  return static_cast<uint32_t>(m < 0 ? m + size : m);
}

inline float decode_texel(uint8_t v, bool srgb) {
  return srgb ? srgb_lut()[v] : static_cast<float>(v) * (1.0f / 255.0f);
}

inline float4 fetch_nearest(const uint8_t* texels, const CpuTexture& tex, uint8_t level, float2 uv) {
  const uint32_t w = mip_size(tex.width, level);
  const uint32_t h = mip_size(tex.height, level);
  const uint8_t* base = texels + tex.base + tex.mip_offsets[level];

  const uint32_t x = wrap_index(static_cast<int>(std::floor(uv.x * static_cast<float>(w))), w, tex.wrap_u);
  const uint32_t y = wrap_index(static_cast<int>(std::floor(uv.y * static_cast<float>(h))), h, tex.wrap_v);

  const uint8_t* texel = base + (static_cast<size_t>(y) * w + x) * kTextureChannels;
  const bool   is_srgb = tex.space == ColorSpace::SRGB;
  
  return float4{
    decode_texel(texel[0], is_srgb),
    decode_texel(texel[1], is_srgb),
    decode_texel(texel[2], is_srgb),
    decode_texel(texel[3], false)
  };
}

inline float4 fetch_bilinear(const uint8_t* texels, const CpuTexture& tex, uint8_t level, float2 uv) {
  const uint32_t w    = mip_size(tex.width, level);
  const uint32_t h    = mip_size(tex.height, level);
  const uint8_t* base = texels + tex.base + tex.mip_offsets[level];

  const float x = uv.x * static_cast<float>(w) - 0.5f;
  const float y = uv.y * static_cast<float>(h) - 0.5f;

  const float fx = x - std::floor(x);
  const float fy = y - std::floor(y);

  const int ix = static_cast<int>(std::floor(x));
  const int iy = static_cast<int>(std::floor(y));

  const uint32_t x0 = wrap_index(ix,     w, tex.wrap_u);
  const uint32_t x1 = wrap_index(ix + 1, w, tex.wrap_u);
  const uint32_t y0 = wrap_index(iy,     h, tex.wrap_v);
  const uint32_t y1 = wrap_index(iy + 1, h, tex.wrap_v);

  const size_t row0 = static_cast<size_t>(y0) * w;
  const size_t row1 = static_cast<size_t>(y1) * w;

  const bool srgb = tex.space == ColorSpace::SRGB;

  float c[4];
  for (uint32_t ch = 0; ch < 4; ++ch) {
    const bool curved = srgb && ch < 3;

    const float t00 = decode_texel(base[(row0 + x0) * kTextureChannels + ch], curved);
    const float t10 = decode_texel(base[(row0 + x1) * kTextureChannels + ch], curved);
    const float t01 = decode_texel(base[(row1 + x0) * kTextureChannels + ch], curved);
    const float t11 = decode_texel(base[(row1 + x1) * kTextureChannels + ch], curved);

    c[ch] = lerp(lerp(t00, t10, fx), lerp(t01, t11, fx), fy);
  }

  return float4{c[0], c[1], c[2], c[3]};
}

inline float4 fetch_trilinear(const uint8_t* texels, const CpuTexture& tex, float level, float2 uv) {
  const uint8_t l0 = static_cast<uint8_t>(level);
  const float t = level - static_cast<float>(l0);
  
  const float4 a = fetch_bilinear(texels, tex, l0, uv);
  if (t <= 0.0f) return a;
  
  const uint8_t l1 = static_cast<uint8_t>(l0 + 1);
  const float4 b = fetch_bilinear(texels, tex, l1, uv);
  return a * (1.0f - t) + b * t;
}


inline float4 sample_texture(const uint8_t* texels, const CpuTexture& tex, float2 uv, float lod) {
  assert(tex.ok() && "sampled a texture the cache never committed");

  const float level = clamp(lod, 0.0f, static_cast<float>(tex.level_count - 1));
  const uint8_t nearest_level = static_cast<uint8_t>(level + 0.5f);

  switch(tex.filter) {
    case TexFilter::Nearest:    return fetch_nearest(texels, tex, nearest_level, uv);
    case TexFilter::Bilinear:   return fetch_bilinear(texels, tex, nearest_level, uv);
    case TexFilter::Trilinear:  return fetch_trilinear(texels, tex, level, uv);
  }

  return fetch_trilinear(texels, tex, level, uv);
}

inline float texture_mip_level(const CpuTexture& tex, float lod) {
  const float texels = static_cast<float>(tex.width) * static_cast<float>(tex.height);
  return lod + 0.5f * std::log2(texels);
}

} // namespace re