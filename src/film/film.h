#pragma once

#include <vector>
#include "core/types.h"

namespace re {

class Film {
public:
  Film(int width, int height);

  void set_pixel(int x, int y, const float3& value);
  const std::vector<float3>& pixels() const { return pixels_; }
  int width() const { return width_; }
  int height() const { return height_; }

private:
  int width_ = 0;
  int height_ = 0;
  std::vector<float3> pixels_;
};

}