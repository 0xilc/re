#include "parallel/scheduler.h"

#include <algorithm>

namespace re {

void parallel_tiles(ThreadPool& pool, int width, int height,
                    const std::function<void(const Tile&)>& body) 
{
  const int tiles_x = (width + kTileSize - 1) / kTileSize;
  const int tiles_y = (height + kTileSize - 1) / kTileSize;
  const int count = tiles_x * tiles_y;

  pool.parallel_for(count, [&](int index) {
    const int tx = index % tiles_x;
    const int ty = index / tiles_x;
    
    Tile tile;
    tile.x0 = tx * kTileSize;
    tile.y0 = ty * kTileSize;
    tile.x1 = std::min(tile.x0 + kTileSize, width);
    tile.y1 = std::min(tile.y0 + kTileSize, height);  

    body(tile);
  });
}

}