#include "core/types.h"

static_assert(sizeof(re::float2) == 8, "Size Assert");
static_assert(alignof(re::float2) == 8, "Align Assert");

static_assert(sizeof(re::float3) == 12, "Size Assert");
static_assert(alignof(re::float3) == 4, "Align Assert");

static_assert(sizeof(re::float4) == 16, "Size Assert");
static_assert(alignof(re::float4) == 16, "Align Assert");
