#include "film/tonemap.h"

namespace re {

namespace {
float3 reinhard(const float3& c) {
    return float3{c.x / (1.0f + c.x), c.y / (1.0f + c.y), c.z / (1.0f + c.z)};
}

float3 map_pixel(const float3& c, Tonemap mapper) {
    switch (mapper) {
        case Tonemap::None:     return c;
        case Tonemap::Reinhard: return reinhard(c);
    }
    return c;
}
}  // namespace

std::vector<float3> tonemap(const Film& film, Tonemap mapper) {
    const std::vector<float3>& src = film.pixels();

    std::vector<float3> out(src.size());
    for (size_t i = 0; i < src.size(); ++i) out[i] = map_pixel(src[i], mapper);
    return out;
}

}  // namespace re