#pragma once

#include <cstdint>
#include <string>

namespace re {

bool write_png(const std::string& path, const uint8_t* rgb, int width, int height);

}