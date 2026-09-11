#include "io/image_io.h"

#include <stb_image_write.h>
#include <stb_image.h>
#include <cmath>

#include "core/log.h"
#include "core/color.h"

namespace re {

Image read_image(const std::string& path) {
  const bool is_hdr = stbi_is_hdr(path.c_str()) != 0;
  int w = 0, h = 0, channels = 0;
  
  Image img;
  if (is_hdr) {
    float* p = stbi_loadf(path.c_str(), &w, &h, &channels, kNumChannels);
    if (!p) {
      log_error("read_image failed: {}", path);
      return {};
    }

    img.width  = static_cast<uint32_t>(w);
    img.height = static_cast<uint32_t>(h);
    img.format = ImageFormat::RGBA32F;

    const size_t size = static_cast<size_t>(w) * static_cast<size_t>(h) * kNumChannels * sizeof(float);
    
    const uint8_t* begin = reinterpret_cast<const uint8_t*>(p);
    img.data.assign(begin, begin + size);

    stbi_image_free(p);
    return img;
  }

  uint8_t* p = stbi_load(path.c_str(), &w, &h, &channels, kNumChannels);
  if (!p) {
    log_error("read_image failed: {}", path);
    return {};
  }

  img.width  = static_cast<uint32_t>(w);
  img.height = static_cast<uint32_t>(h);
  img.format = ImageFormat::RGBA8;

  const size_t size =
      static_cast<size_t>(w) *
      static_cast<size_t>(h) *
      kNumChannels;

  img.data.assign(p, p + size);

  stbi_image_free(p);
  return img;
}

Image read_image(const uint8_t* bytes, size_t n) {
  if (!bytes || n == 0) return {};

  const int len = static_cast<int>(n);
  const bool is_hdr = stbi_is_hdr_from_memory(bytes, len) != 0;
  int w = 0, h = 0, channels = 0;

  Image img;
  if (is_hdr) {
    float* p =
        stbi_loadf_from_memory(bytes, len, &w, &h, &channels, kNumChannels);

    if (!p) {
      log_error("read_image embedded failed");
      return {};
    }

    img.width  = static_cast<uint32_t>(w);
    img.height = static_cast<uint32_t>(h);
    img.format = ImageFormat::RGBA32F;

    const size_t size = static_cast<size_t>(w) * static_cast<size_t>(h) * kNumChannels * sizeof(float);
    
    const uint8_t* begin = reinterpret_cast<const uint8_t*>(p);
    img.data.assign(begin, begin + size);

    stbi_image_free(p);
    return img;
  }

  uint8_t* p =
      stbi_load_from_memory(bytes, len, &w, &h, &channels, kNumChannels);

  if (!p) {
    log_error("read_image embedded failed");
    return {};
  }

  img.width  = static_cast<uint32_t>(w);
  img.height = static_cast<uint32_t>(h);
  img.format = ImageFormat::RGBA8;

  const size_t size = static_cast<size_t>(w) * static_cast<size_t>(h) * kNumChannels;
  img.data.assign(p, p + size);

  stbi_image_free(p);
  return img;
}

std::vector<uint8_t> encode_srgb8(const std::vector<float3>& linear) {
    std::vector<uint8_t> out(linear.size() * 3);

    for (size_t i = 0; i < linear.size(); ++i) {
        const float3& c = linear[i];
        out[i * 3 + 0] = static_cast<uint8_t>(linear_to_srgb(c.x) * 255.0f + 0.5f);
        out[i * 3 + 1] = static_cast<uint8_t>(linear_to_srgb(c.y) * 255.0f + 0.5f);
        out[i * 3 + 2] = static_cast<uint8_t>(linear_to_srgb(c.z) * 255.0f + 0.5f);
    }

    return out;
}

bool write_png(const std::string& path, const uint8_t* rgb, int width, int height) {
    const int stride_bytes = width * 3;
    const int ok = stbi_write_png(path.c_str(), width, height, 3, rgb, stride_bytes);

    if (ok == 0) {
        log_error("write_png failed: {}", path);
        return false;
    }
    return true;
}

}  // namespace re