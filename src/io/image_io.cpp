#include "io/image_io.h"
#include <stb_image_write.h>
#include "core/log.h"

namespace re {

bool write_png(const std::string& path, const uint8_t* rgb, int width, int height) {
  const int stride_bytes = width * 3;
  const int ok = stbi_write_png(path.c_str(), width, height, 3, rgb, stride_bytes);

  if (ok == 0) {
    log_error("write_png failed: {}", path);
    return false;
  }
  return true;
}

}