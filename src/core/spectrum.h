#pragma once

#include "core/platform.h"
#include "core/types.h"

namespace re {

using Spectrum = float3;

RE_HD inline Spectrum make_empty_spectrum() {
  return Spectrum{0.0f, 0.0f, 0.0f};
}

}