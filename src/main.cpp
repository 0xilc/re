#include "config/config.h"
#include "config/render_settings.h"
#include "core/log.h"
#include "driver/offline_driver.h"

int main() {
  re::log_info("re, re, raaaw!");
  re::OfflineSettings driver_settings{
    .width = 512,
    .height = 512,
    .fov_y_deg = 45.0f
  };
  return re::run_offline(driver_settings)? 0 : 1;
}