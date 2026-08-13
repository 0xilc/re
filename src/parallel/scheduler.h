#pragma once

#include <functional>
#include "parallel/thread_pool.h"

namespace re {

constexpr int kTileSize = 16;

struct Tile {
  int x0, y0;
  int x1, y1;
};

void parallel_tiles(ThreadPool& pool, int width, int height,
                    const std::function<void(const Tile&)>& body);

}