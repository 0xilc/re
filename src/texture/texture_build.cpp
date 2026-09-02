#include "texture/texture_build.h"

#include <algorithm>
#include <utility>

#include "core/log.h"
#include "core/math.h"
#include "io/image_io.h"

namespace re {
namespace {

float decode(uint8_t v, bool srgb) {
  return srgb ? srgb_lut()[v] : static_cast<float>(v) * (1.0f / 255.0f);
}

uint8_t encode(float v, bool srgb) {
  const float encoded = srgb ? linear_to_srgb(v) : v;
  return static_cast<uint8_t>(clamp(encoded, 0.0f, 1.0f) * 255.0f + 0.5f);
}

uint32_t level_count(uint32_t width, uint32_t height) {
  uint32_t levels = 1;
  while (mip_size(width, levels - 1) > 1 || mip_size(height, levels - 1) > 1) ++levels;
  return levels;
}

std::vector<uint8_t> downsample(const std::vector<uint8_t>& src, uint32_t src_w, uint32_t src_h,
                                uint32_t dst_w, uint32_t dst_h, bool srgb) {
  std::vector<uint8_t> dst(static_cast<size_t>(dst_w) * dst_h * kTextureChannels);

  for (uint32_t y = 0; y < dst_h; ++y) {
    const uint32_t y0 = std::min(2 * y, src_h - 1);
    const uint32_t y1 = std::min(y0 + 1, src_h - 1);

    for (uint32_t x = 0; x < dst_w; ++x) {
      const uint32_t x0 = std::min(2 * x, src_w - 1);
      const uint32_t x1 = std::min(x0 + 1, src_w - 1);

      const size_t taps[4] = {
          (static_cast<size_t>(y0) * src_w + x0) * kTextureChannels,
          (static_cast<size_t>(y0) * src_w + x1) * kTextureChannels,
          (static_cast<size_t>(y1) * src_w + x0) * kTextureChannels,
          (static_cast<size_t>(y1) * src_w + x1) * kTextureChannels,
      };

      const size_t out = (static_cast<size_t>(y) * dst_w + x) * kTextureChannels;
      for (uint32_t c = 0; c < kTextureChannels; ++c) {
        const bool curved = srgb && c < 3;

        float sum = 0.0f;
        for (const size_t tap : taps) sum += decode(src[tap + c], curved);

        dst[out + c] = encode(sum * 0.25f, curved);
      }
    }
  }

  return dst;
}

void flip_green(std::vector<uint8_t>& rgba) {
  for (size_t i = 1; i < rgba.size(); i += kTextureChannels) {
    rgba[i] = static_cast<uint8_t>(255 - rgba[i]);
  }
}

void build_mip_chain(TextureImage& tex) {
  if (!tex.ok() || tex.width == 0 || tex.height == 0) return;

  tex.levels.resize(1);

  const uint32_t levels = level_count(tex.width, tex.height);
  const bool     srgb   = tex.space == ColorSpace::SRGB;

  for (uint32_t l = 1; l < levels; ++l) {
    tex.levels.push_back(downsample(tex.levels[l - 1],
                                    mip_size(tex.width, l - 1), mip_size(tex.height, l - 1),
                                    mip_size(tex.width, l), mip_size(tex.height, l), srgb));
  }
}

}  // namespace

TextureImage build_texture(const TextureDesc& d) {
  ImageData raw = d.bytes.empty() ? read_image(d.path)
                                  : read_image(d.bytes.data(), d.bytes.size());

  if (!raw.ok()) {
    log_error("texture: could not decode {}", d.name.empty() ? d.path : d.name);
    return {};
  }

  const size_t expected = static_cast<size_t>(raw.width) * raw.height * kTextureChannels;
  if (raw.pixels.size() != expected) {
    log_error("texture: {} decoded to {} bytes, expected {}", d.name, raw.pixels.size(), expected);
    return {};
  }

  TextureImage out;
  out.width = raw.width;
  out.height = raw.height;
  out.space = d.color_space;
  out.wrap_u = d.wrap_u;
  out.wrap_v = d.wrap_v;
  out.is_normal_map = d.is_normal_map;
  out.levels.push_back(std::move(raw.pixels));

  if (d.is_normal_map) flip_green(out.levels[0]);
  build_mip_chain(out);

  return out;
}

}
