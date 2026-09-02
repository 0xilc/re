#include "io/image_io.h"

#include <stb_image_write.h>
#include <cmath>
#include <stb_image.h>

#include "core/log.h"
#include "core/color.h"

namespace re {
namespace {
// stbi_failure_reason() is null until stb has actually failed once.
const char* failure_reason() {
  const char* reason = stbi_failure_reason();
  return reason ? reason : "unknown";
}

ImageData finish(uint8_t* p, int w, int h) {
  ImageData img;
  if (!p) return img;
  img.width  = static_cast<uint32_t>(w);
  img.height = static_cast<uint32_t>(h);
  img.pixels.assign(p, p + static_cast<size_t>(w) * h * 4);
  stbi_image_free(p);
  return img;
}
}  // namespace

ImageData read_image(const std::string& path) {
  int w = 0, h = 0, n = 0;
  uint8_t* p = stbi_load(path.c_str(), &w, &h, &n, 4);   // 4 = RGBA'ya zorla
  if (!p) log_error("decode_image failed: {} ({})", path, failure_reason());
  return finish(p, w, h);
}

ImageData read_image(const uint8_t* bytes, size_t n) {
  if (!bytes || n == 0) return {};
  int w = 0, h = 0, ch = 0;
  uint8_t* p = stbi_load_from_memory(bytes, static_cast<int>(n), &w, &h, &ch, 4);
  if (!p) log_error("decode_image (embedded) failed: {}", failure_reason());
  return finish(p, w, h);
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