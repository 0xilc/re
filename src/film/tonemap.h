#pragma once

#include "film/film.h"
#include <cstdint>
#include <vector>

namespace re {

float linear_to_srgb(float linear);
std::vector<uint8_t> encode_srgb8(const Film& film);

}