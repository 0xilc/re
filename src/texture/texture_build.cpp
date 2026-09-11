#include "texture/texture_build.h"

#include <algorithm>
#include <utility>
#include <cstring>

#include "core/log.h"
#include "core/math.h"
#include "io/image_io.h"

namespace re {
namespace {

size_t texel_size(const ImageFormat cf) {
  switch (cf) {
    case re::ImageFormat::RGBA8:   return 4 * sizeof(uint8_t);
    case re::ImageFormat::RGBA32F: return 4 * sizeof(float);
  }

  return 0;
}

float4 load_texel(const std::vector<uint8_t>& data,
                  size_t texel,
                  ImageFormat format,
                  ColorSpace space) {
  float4 value;

  switch (format) {
    case ImageFormat::RGBA8: {
      const uint8_t* p = data.data() + texel * 4;

      if (space == ColorSpace::SRGB) {
        value.x = srgb_lut()[p[0]];
        value.y = srgb_lut()[p[1]];
        value.z = srgb_lut()[p[2]];
      } else {
        value.x = static_cast<float>(p[0]) / 255.0f;
        value.y = static_cast<float>(p[1]) / 255.0f;
        value.z = static_cast<float>(p[2]) / 255.0f;
      }

      value.w = static_cast<float>(p[3]) / 255.0f;
      return value;
    }

    case ImageFormat::RGBA32F: {
      float rgba[4];
      std::memcpy(rgba, data.data() + texel * 4 * sizeof(float), sizeof(rgba));

      value = float4{rgba[0], rgba[1], rgba[2], rgba[3]};

      if (space == ColorSpace::SRGB) {
        value.x = srgb_to_linear(value.x);
        value.y = srgb_to_linear(value.y);
        value.z = srgb_to_linear(value.z);
      }

      return value;
    }
  }

  return {};
}

void store_texel(std::vector<uint8_t>& data,
                 size_t texel,
                 ImageFormat format,
                 ColorSpace space,
                 float4 value)
{
  if (space == ColorSpace::SRGB) {
    value.x = linear_to_srgb(value.x);
    value.y = linear_to_srgb(value.y);
    value.z = linear_to_srgb(value.z);
  }

  switch (format) {
    case ImageFormat::RGBA8: {
      uint8_t* p = data.data() + texel * 4;

      p[0] = static_cast<uint8_t>(clamp(value.x, 0.0f, 1.0f) * 255.0f + 0.5f);
      p[1] = static_cast<uint8_t>(clamp(value.y, 0.0f, 1.0f) * 255.0f + 0.5f);
      p[2] = static_cast<uint8_t>(clamp(value.z, 0.0f, 1.0f) * 255.0f + 0.5f);
      p[3] = static_cast<uint8_t>(clamp(value.w, 0.0f, 1.0f) * 255.0f + 0.5f);
      return;
    }

    case ImageFormat::RGBA32F: {
      const float rgba[4] = {value.x, value.y, value.z, value.w};
      std::memcpy(data.data() + texel * 4 * sizeof(float), rgba, sizeof(rgba));
      return;
    }
  }
}


uint32_t level_count(uint32_t width, uint32_t height) {
  uint32_t levels = 1;
  
  while (mip_size(width, levels - 1) > 1 || mip_size(height, levels - 1) > 1) ++levels;
  
  return levels;
}

std::vector<uint8_t> build_mip_level(const TextureImage& tex, uint32_t level) {
  const std::vector<uint8_t>& src = tex.levels[level - 1];

  const uint32_t src_w = mip_size(tex.width, level - 1);
  const uint32_t src_h = mip_size(tex.height, level - 1);
  const uint32_t dst_w = mip_size(tex.width, level);
  const uint32_t dst_h = mip_size(tex.height, level);

  std::vector<uint8_t> dst(
      static_cast<size_t>(dst_w) * dst_h * texel_size(tex.format));

  for (uint32_t y = 0; y < dst_h; ++y) {
    const uint32_t y0 = std::min(2 * y,     src_h - 1);
    const uint32_t y1 = std::min(2 * y + 1, src_h - 1);

    for (uint32_t x = 0; x < dst_w; ++x) {
      const uint32_t x0 = std::min(2 * x,     src_w - 1);
      const uint32_t x1 = std::min(2 * x + 1, src_w - 1);

      const size_t i00 = static_cast<size_t>(y0) * src_w + x0;
      const size_t i10 = static_cast<size_t>(y0) * src_w + x1;
      const size_t i01 = static_cast<size_t>(y1) * src_w + x0;
      const size_t i11 = static_cast<size_t>(y1) * src_w + x1;

      const float4 a = load_texel(src, i00, tex.format, tex.space);
      const float4 b = load_texel(src, i10, tex.format, tex.space);
      const float4 c = load_texel(src, i01, tex.format, tex.space);
      const float4 d = load_texel(src, i11, tex.format, tex.space);

      const float4 average = (a + b + c + d) * 0.25f;

      const size_t out = static_cast<size_t>(y) * dst_w + x;
      store_texel(dst, out, tex.format, tex.space, average);
    }
  }

  return dst;
}

void build_mip_chain(TextureImage& tex) {
  if (!tex.ok()) return;

  tex.levels.resize(1);

  const uint32_t num_levels = level_count(tex.width, tex.height);

  for (uint32_t level = 1; level < num_levels; ++level) {
    tex.levels.push_back(build_mip_level(tex, level));
  }
}

}  // namespace

TextureImage build_texture(const TextureDesc& d) {
  Image raw = d.bytes.empty() ? read_image(d.path)
                              : read_image(d.bytes.data(), d.bytes.size());

  if (!raw.ok()) {
    log_error("texture: could not decode {}", d.name.empty() ? d.path : d.name);
    return {};
  }

  TextureImage out;
  out.width = raw.width;
  out.height = raw.height;
  out.space = d.color_space;
  out.wrap_u = d.wrap_u;
  out.wrap_v = d.wrap_v;
  out.filter = d.filter;
  out.format = raw.format;
  out.levels.push_back(std::move(raw.data));
  
  build_mip_chain(out);

  return out;
}

}
