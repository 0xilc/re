#pragma once

#include "film/film.h"
#include <cstdint>
#include <vector>

namespace re {

enum class Tonemap : uint8_t {
  None,
  Reinhard
};

float3 tonemap(const float3& c, Tonemap mapper);

float linear_to_srgb(float linear);

std::vector<uint8_t> encode_srgb8(const Film& film);

}