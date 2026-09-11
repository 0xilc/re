#pragma once 

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>

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

inline float4 load_texel_rgba8(const uint8_t* p, ColorSpace space) {
  constexpr float kInv255 = 1.0f / 255.0f;

  if (space == ColorSpace::SRGB) {
    const float* lut = srgb_lut();

    return float4{
        lut[p[0]],
        lut[p[1]],
        lut[p[2]],
        static_cast<float>(p[3]) * kInv255,
    };
  }

  return float4{
      static_cast<float>(p[0]) * kInv255,
      static_cast<float>(p[1]) * kInv255,
      static_cast<float>(p[2]) * kInv255,
      static_cast<float>(p[3]) * kInv255,
  };
}

inline float4 load_texel_rgba32f(const uint8_t* p, ColorSpace space) {
  float rgba[4];
  std::memcpy(rgba, p, sizeof(rgba));

  float4 value{rgba[0], rgba[1], rgba[2], rgba[3]};

  if (space == ColorSpace::SRGB) {
    value.x = srgb_to_linear(value.x);
    value.y = srgb_to_linear(value.y);
    value.z = srgb_to_linear(value.z);
  }

  return value;
}

inline float4 load_texel(const uint8_t* base,
                         size_t texel_index,
                         const CpuTexture& tex) 
{
  const uint8_t* p = base + texel_index * texel_size(tex.format);

  switch (tex.format) {
    case ImageFormat::RGBA8:
      return load_texel_rgba8(p, tex.space);

    case ImageFormat::RGBA32F:
      return load_texel_rgba32f(p, tex.space);
  }

  return {};
}

inline float4 fetch_nearest(const uint8_t* texels,
                            const CpuTexture& tex,
                            uint8_t level,
                            float2 uv) 
{
  const uint32_t w = mip_size(tex.width, level);
  const uint32_t h = mip_size(tex.height, level);

  const uint8_t* base =
      texels + tex.base + tex.mip_offsets[level];

  const uint32_t x = wrap_index(
      static_cast<int>(std::floor(uv.x * static_cast<float>(w))),
      w,
      tex.wrap_u);

  const uint32_t y = wrap_index(
      static_cast<int>(std::floor(uv.y * static_cast<float>(h))),
      h,
      tex.wrap_v);

  const size_t index =
      static_cast<size_t>(y) * w + x;

  return load_texel(base, index, tex);
}

inline float4 fetch_bilinear(const uint8_t* texels,
                             const CpuTexture& tex,
                             uint8_t level,
                             float2 uv) 
{
  const uint32_t w = mip_size(tex.width, level);
  const uint32_t h = mip_size(tex.height, level);

  const uint8_t* base =
      texels + tex.base + tex.mip_offsets[level];

  const float x = uv.x * static_cast<float>(w) - 0.5f;
  const float y = uv.y * static_cast<float>(h) - 0.5f;

  const int ix = static_cast<int>(std::floor(x));
  const int iy = static_cast<int>(std::floor(y));

  const float fx = x - std::floor(x);
  const float fy = y - std::floor(y);

  const uint32_t x0 = wrap_index(ix,     w, tex.wrap_u);
  const uint32_t x1 = wrap_index(ix + 1, w, tex.wrap_u);
  const uint32_t y0 = wrap_index(iy,     h, tex.wrap_v);
  const uint32_t y1 = wrap_index(iy + 1, h, tex.wrap_v);

  const size_t i00 = static_cast<size_t>(y0) * w + x0;
  const size_t i10 = static_cast<size_t>(y0) * w + x1;
  const size_t i01 = static_cast<size_t>(y1) * w + x0;
  const size_t i11 = static_cast<size_t>(y1) * w + x1;

  const float4 c00 = load_texel(base, i00, tex);
  const float4 c10 = load_texel(base, i10, tex);
  const float4 c01 = load_texel(base, i01, tex);
  const float4 c11 = load_texel(base, i11, tex);

  const float4 cx0 = lerp(c00, c10, fx);
  const float4 cx1 = lerp(c01, c11, fx);

  return lerp(cx0, cx1, fy);
}

inline float4 fetch_trilinear(const uint8_t* texels,
                              const CpuTexture& tex,
                              float level,
                              float2 uv) 
{
  const uint8_t l0 = static_cast<uint8_t>(level);
  const float t = level - static_cast<float>(l0);

  const float4 a = fetch_bilinear(texels, tex, l0, uv);
  if (t <= 0.0f) return a;

  const uint8_t l1 = static_cast<uint8_t>(l0 + 1);
  const float4 b = fetch_bilinear(texels, tex, l1, uv);

  return lerp(a, b, t);
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