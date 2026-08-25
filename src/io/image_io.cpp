#include "io/image_io.h"

#include <stb_image_write.h>
#include <cmath>

#include "core/log.h"
#include "core/color.h"

namespace re {

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