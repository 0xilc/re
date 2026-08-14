#include "film/tonemap.h"
#include "core/math.h"
#include <stdio.h>

namespace re {

namespace {
// --- Reinhard -------
float3 reinhard(const float3& c) {
    return float3{c.x / (1.0f + c.x), c.y / (1.0f + c.y), c.z / (1.0f + c.z)};
}

// --- AgX -------
// 
// Minimal AgX implementation 
// Uses the 6th order polynomial approximation
float3 agx_inset(const float3& c) {
  return float3{
    0.842479062f * c.x +
    0.078433600f * c.y +
    0.079223745f * c.z,

    0.042328242f * c.x +
    0.878468636f * c.y +
    0.079166127f * c.z,

    0.042375655f * c.x +
    0.078433600f * c.y +
    0.879142974f * c.z
  };
}

float3 agx_outset(const float3& c) {
  return float3{
    1.196879005f * c.x -
    0.098020881f * c.y -
    0.099029744f * c.z,

  -0.052896852f * c.x +
    1.151903130f * c.y -
    0.098961177f * c.z,

  -0.052971635f * c.x -
    0.098043450f * c.y +
    1.151073672f * c.z
  };
}

float agx_default_contrast(float x){
  const float x2 = x * x;
  const float x4 = x2 * x2;

  return
      15.5f    * x4 * x2
    - 40.14f   * x4 * x
    + 31.96f   * x4
    - 6.868f   * x2 * x
    + 0.4298f  * x2
    + 0.1191f  * x
    - 0.00232f;
}

float3 agx(float3 c) {
  constexpr float kMinEv = -12.47393f;
  constexpr float kMaxEv = 4.026069f;

  c = agx_inset(c);

  c.x = log2f(fmaxf(c.x, 1e-10f));
  c.y = log2f(fmaxf(c.y, 1e-10f));
  c.z = log2f(fmaxf(c.z, 1e-10f));

  c.x = fmaxf(kMinEv, fminf(kMaxEv, c.x));
  c.y = fmaxf(kMinEv, fminf(kMaxEv, c.y));
  c.z = fmaxf(kMinEv, fminf(kMaxEv, c.z));

  constexpr float kInvEvRange = 1.0f / (kMaxEv - kMinEv);
  c.x = (c.x - kMinEv) * kInvEvRange;
  c.y = (c.y - kMinEv) * kInvEvRange;
  c.z = (c.z - kMinEv) * kInvEvRange;

  c.x = agx_default_contrast(c.x);
  c.y = agx_default_contrast(c.y);
  c.z = agx_default_contrast(c.z);

  c = agx_outset(c);

  c.x = fmaxf(0.0f, fminf(1.0f, c.x));
  c.y = fmaxf(0.0f, fminf(1.0f, c.y));
  c.z = fmaxf(0.0f, fminf(1.0f, c.z));

  return c;
};

// --- util -------
float3 map_pixel(const float3& c, Tonemap mapper) {
    switch (mapper) {
        case Tonemap::None:     return c;
        case Tonemap::Reinhard: return reinhard(c);
        case Tonemap::AgX:      return agx(c);
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