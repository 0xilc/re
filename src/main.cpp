#include "config/config.h"
#include "config/render_settings.h"
#include "core/log.h"
#include "driver/offline_driver.h"

int main() {
  re::log_info("re, re, raaaw!");
  re::OfflineSettings driver_settings;
  return re::run_offline(driver_settings)? 0 : 1;
}