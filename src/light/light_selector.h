#pragma once

#include <concepts>

#include "core/types.h"

namespace re {

struct LightRef {
  int inst_id;
  int prim_id;
};

struct LightSampleContext {
  float3 position;
  float3 normal;
};

struct LightPick {
  LightRef ref;
  float    pmf;
};

template <typename L>
concept LightSelector = requires(const L& lights, const LightSampleContext& ctx,
                                float u, LightRef ref) {
  { lights.pick(ctx, u) } -> std::same_as<LightPick>;
  { lights.pmf(ctx, ref) }  -> std::same_as<float>;
};

}
