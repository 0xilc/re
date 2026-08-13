#include "film/film.h"

#include <cassert>
#include <cmath>

namespace re {

namespace {
bool is_finite(const float3& c) {
  return std::isfinite(c.x) && std::isfinite(c.y) && std::isfinite(c.z);
}
}

Film::Film(int width, int height) 
  : width_(width), height_(height), pixels_(static_cast<size_t>(width * height)) {
  assert(width > 0 && height > 0);
}

void Film::set_pixel(int x, int y, const float3& value) {
  assert(x >= 0 && x < width_ && y >= 0 && y < height_);

  if (!is_finite(value)) {
    assert(false && "non-finite sample written to film");
    ++nonfinite_count_; 
    return;
  }

  pixels_[static_cast<size_t>(y * width_ + x)] = value;
}

}