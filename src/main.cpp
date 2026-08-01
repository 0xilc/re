#include "config/config.h"
#include "config/render_settings.h"
#include "core/log.h"

int main() {
  re::log_info("re, re, raaaw!");

  std::string config_path = "configs/default.toml";
  auto settings_opt = re::load_settings(config_path);
  if (!settings_opt.has_value()) {
    return 1;
  }
  re::RenderSettings& settings = settings_opt.value();
  
  re::log_info("Resolution: {} x {}", settings.width, settings.height);
  re::log_info("Spp: {}", settings.spp);
  re::log_info("Output path: {}", settings.output_path);
  return 0;
}