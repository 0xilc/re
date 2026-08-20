#pragma once

#include <concepts>

#include "core/platform.h"
#include "core/types.h"

namespace re {

struct LightRef {
  int instance_id;
  int prim_id;
};

struct LightSampleContext {
  float3 position;
  float3 normal;
};

struct SampledLight {
  LightRef ref;
  float    pmf;
};

template <typename L>
concept LightSampler = requires(const L& lights, const LightSampleContext& ctx,
                                float u, LightRef ref) {
  { lights.sample(ctx, u) } -> std::same_as<SampledLight>;
  { lights.pmf(ctx, ref) }  -> std::same_as<float>;
};

}
