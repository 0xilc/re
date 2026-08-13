// Usage: image_diff <a.png> <b.png> [heatmap.png]

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include "stb_image.h"
#include "stb_image_write.h"

namespace {

float srgb_to_linear(unsigned char v) {
    const float s = v / 255.0f;
    return s <= 0.04045f ? s / 12.92f : std::pow((s + 0.055f) / 1.055f, 2.4f);
}

unsigned char linear_to_srgb_byte(float linear) {
    linear = linear > 0.0f ? (linear < 1.0f ? linear : 1.0f) : 0.0f;
    const float s = linear <= 0.0031308f ? 12.92f * linear
                                         : 1.055f * std::pow(linear, 1.0f / 2.4f) - 0.055f;
    return static_cast<unsigned char>(s * 255.0f + 0.5f);
}

void heat_colour(float t, float& r, float& g, float& b) {
    t = t > 0.0f ? (t < 1.0f ? t : 1.0f) : 0.0f;
    if (t < 0.25f) {
        const float u = t * 4.0f;
        r = 0.0f; g = 0.0f; b = u;
    } else if (t < 0.5f) {
        const float u = (t - 0.25f) * 4.0f;
        r = 0.0f; g = u; b = 1.0f - u;
    } else if (t < 0.75f) {
        const float u = (t - 0.5f) * 4.0f;
        r = u; g = 1.0f; b = 0.0f;
    } else {
        const float u = (t - 0.75f) * 4.0f;
        r = 1.0f; g = 1.0f - u; b = 0.0f;
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 3) {
        std::printf("usage: image_diff <a.png> <b.png> [heatmap.png]\n");
        return 2;
    }

    int aw, ah, ac;
    int bw, bh, bc;
    unsigned char* a = stbi_load(argv[1], &aw, &ah, &ac, 3);
    if (a == nullptr) {
        std::printf("could not read %s\n", argv[1]);
        return 2;
    }
    unsigned char* b = stbi_load(argv[2], &bw, &bh, &bc, 3);
    if (b == nullptr) {
        std::printf("could not read %s\n", argv[2]);
        stbi_image_free(a);
        return 2;
    }

    if (aw != bw || ah != bh) {
        std::printf("size mismatch: %dx%d vs %dx%d\n", aw, ah, bw, bh);
        stbi_image_free(a);
        stbi_image_free(b);
        return 2;
    }

    const size_t pixel_count = static_cast<size_t>(aw) * ah;

    double sum_sq   = 0.0;
    double sum_a    = 0.0;
    double sum_b    = 0.0;
    double max_diff = 0.0;
    int    max_x = 0, max_y = 0;

    std::vector<float> per_pixel(pixel_count, 0.0f);

    for (size_t i = 0; i < pixel_count; ++i) {
        double pixel_sq = 0.0;
        for (int c = 0; c < 3; ++c) {
            const float la = srgb_to_linear(a[i * 3 + c]);
            const float lb = srgb_to_linear(b[i * 3 + c]);
            const double d = static_cast<double>(la) - lb;

            pixel_sq += d * d;
            sum_a += la;
            sum_b += lb;
        }
        sum_sq += pixel_sq;

        const double pixel_rms = std::sqrt(pixel_sq / 3.0);
        per_pixel[i] = static_cast<float>(pixel_rms);

        if (pixel_rms > max_diff) {
            max_diff = pixel_rms;
            max_x    = static_cast<int>(i % static_cast<size_t>(aw));
            max_y    = static_cast<int>(i / static_cast<size_t>(aw));
        }
    }

    const double rmse    = std::sqrt(sum_sq / (pixel_count * 3));
    const double mean_a  = sum_a / (pixel_count * 3);
    const double mean_b  = sum_b / (pixel_count * 3);

    std::printf("rmse        %.6f\n", rmse);
    std::printf("max diff    %.6f  at (%d, %d)\n", max_diff, max_x, max_y);
    std::printf("mean a      %.6f\n", mean_a);
    std::printf("mean b      %.6f\n", mean_b);
    std::printf("mean ratio  %.6f\n", mean_a > 0.0 ? mean_b / mean_a : 0.0);

    if (argc >= 4) {
        const float scale = max_diff > 0.0 ? static_cast<float>(1.0 / max_diff) : 0.0f;

        std::vector<unsigned char> out(pixel_count * 3);
        for (size_t i = 0; i < pixel_count; ++i) {
            float r, g, bl;
            heat_colour(per_pixel[i] * scale, r, g, bl);
            out[i * 3 + 0] = linear_to_srgb_byte(r);
            out[i * 3 + 1] = linear_to_srgb_byte(g);
            out[i * 3 + 2] = linear_to_srgb_byte(bl);
        }
        if (stbi_write_png(argv[3], aw, ah, 3, out.data(), aw * 3) == 0) {
            std::printf("could not write %s\n", argv[3]);
        } else {
            std::printf("heatmap     %s  (normalised to max diff)\n", argv[3]);
        }
    }

    stbi_image_free(a);
    stbi_image_free(b);
    return 0;
}