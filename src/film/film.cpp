#include "film/film.h"

#include <cassert>

namespace re {

Film::Film(int width, int height) 
  : width_(width), height_(height), pixels_(static_cast<size_t>(width * height)) {
  assert(width > 0 && height > 0);
}

void Film::set_pixel(int x, int y, const float3& value) {
  assert(x >= 0 && x < width_ && y >= 0 && y < height_);
  pixels_[static_cast<size_t>(y * width_ + x)] = value;
}

}