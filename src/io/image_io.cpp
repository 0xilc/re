#include "io/image_io.h"

#include <stb_image_write.h>
#include <cmath>

#include "core/log.h"

namespace re {

float linear_to_srgb(float linear) {
    linear = linear > 0.0f ? (linear < 1.0f ? linear : 1.0f) : 0.0f;

    return linear <= 0.0031308f ? 12.92f * linear
                                : 1.055f * std::pow(linear, 1.0f / 2.4f) - 0.055f;
}

float srgb_to_linear(float srgb) {
    srgb = srgb > 0.0f ? (srgb < 1.0f ? srgb : 1.0f) : 0.0f;

    return srgb <= 0.04045f ? srgb / 12.92f : std::pow((srgb + 0.055f) / 1.055f, 2.4f);
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