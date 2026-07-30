#include "core/types.h"
#include "core/spectrum.h"

static_assert(sizeof(re::float2) == 8, "Size Assert");
static_assert(alignof(re::float2) == 8, "Align Assert");

static_assert(sizeof(re::float3) == 12, "Size Assert");
static_assert(alignof(re::float3) == 4, "Align Assert");

static_assert(sizeof(re::float4) == 16, "Size Assert");
static_assert(alignof(re::float4) == 16, "Align Assert");

// Spectrum
static_assert(sizeof(re::Spectrum) == 12, "Spectrum(RGB) size must be 12 bytes.");
static_assert(alignof(re::Spectrum) == 4, "Spectrum(RGB) alignment must be 4 bytes.");