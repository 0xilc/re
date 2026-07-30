#include "core/types.h"
#include "core/spectrum.h"
#include "core/ray.h"
#include "core/hit.h"
#include "core/aabb.h"

static_assert(sizeof(re::float2) == 8, "Size Assert");
static_assert(alignof(re::float2) == 8, "Align Assert");

static_assert(sizeof(re::float3) == 12, "Size Assert");
static_assert(alignof(re::float3) == 4, "Align Assert");

static_assert(sizeof(re::float4) == 16, "Size Assert");
static_assert(alignof(re::float4) == 16, "Align Assert");

// Spectrum
static_assert(sizeof(re::Spectrum) == 12, "Spectrum(RGB) size must be 12 bytes.");
static_assert(alignof(re::Spectrum) == 4, "Spectrum(RGB) alignment must be 4 bytes.");

// Ray
static_assert(sizeof(re::Ray) == 32, "Size assert");
static_assert(alignof(re::Ray) == 32, "Align assert");

// Hit
static_assert(sizeof(re::Hit) == 32, "Size must be padded to 32.");
static_assert(alignof(re::Hit) == 32, "Alignment must be padded to 32.");

// AABB
static_assert(sizeof(re::AABB) == 24, "Size of AABB");