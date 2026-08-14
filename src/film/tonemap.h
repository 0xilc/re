#pragma once

#include "film/film.h"
#include <cstdint>
#include <vector>

namespace re {

enum class Tonemap : uint8_t {
  None,
  Reinhard
};

std::vector<float3> tonemap(const Film& c, Tonemap mapper);
}