#pragma once 

#include <cmath>
#include <cstdint>

#include "core/color.h"
#include "core/types.h"
#include "core/math.h"
#include "texture/texture_build.h"
#include "render/cpu/texture_cache.h"

namespace re {

inline int wrap_index(int i, int n, WrapMode mode) {
  if (mode == WrapMode::Clamp) return i < 0 ? 0 : (i >= n ? n - 1 : i);

  const int m = i % n;
  return m < 0 ? m + n : m;
}

inline float decode_texel(uint8_t v, bool srgb) {
  return srgb ? srgb_lut()[v] : static_cast<float>(v) * (1.0f / 255.0f);
}

inline float4 fetch_nearest(const uint8_t* texels, const CpuTexture& tex, uint8_t level, float2 uv) {
  const int w = static_cast<int>(mip_size(tex.width, level));
  const int h = static_cast<int>(mip_size(tex.height, level));
  const uint8_t* base = texels + tex.base + tex.mip_offsets[level];

  const int x = wrap_index(static_cast<int>(std::floor(uv.x * w)), w, tex.wrap_u);
  const int y = wrap_index(static_cast<int>(std::floor(uv.y * h)), h, tex.wrap_v);
  
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
  const int      w    = static_cast<int>(mip_size(tex.width, level));
  const int      h    = static_cast<int>(mip_size(tex.height, level));
  const uint8_t* base = texels + tex.base + tex.mip_offsets[level];

  const float x = uv.x * w - 0.5f;
  const float y = uv.y * h - 0.5f;

  const float fx = x - std::floor(x);
  const float fy = y - std::floor(y);

  const int ix = static_cast<int>(std::floor(x));
  const int iy = static_cast<int>(std::floor(y));

  const int x0 = wrap_index(ix,     w, tex.wrap_u);
  const int x1 = wrap_index(ix + 1, w, tex.wrap_u);
  const int y0 = wrap_index(iy,     h, tex.wrap_v);
  const int y1 = wrap_index(iy + 1, h, tex.wrap_v);

  const size_t row0 = static_cast<size_t>(y0) * w;
  const size_t row1 = static_cast<size_t>(y1) * w;

  const bool srgb = tex.space == ColorSpace::SRGB;

  float c[4];
  for (int ch = 0; ch < 4; ++ch) {
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
  if (!tex.ok()) return float4{1.0f, 0.0f, 1.0f, 1.0f};
  
  const float level = clamp(lod, 0.0f, static_cast<float>(tex.level_count - 1));
  const uint8_t nearest_level = static_cast<uint8_t>(level + 0.5f);

  switch(tex.filter) {
    case TexFilter::Nearest:    return fetch_nearest(texels, tex, nearest_level, uv);
    case TexFilter::Bilinear:   return fetch_bilinear(texels, tex, nearest_level, uv);
    case TexFilter::Trilinear:  return fetch_trilinear(texels, tex, level, uv);
  }

  return fetch_trilinear(texels, tex, level, uv);
}
} // namespace re