#include "config/config.h"
#include "config/render_settings.h"
#include "core/log.h"
#include <toml.hpp>
#include <string>

namespace re {

namespace {

bool read_render_tonemap(toml::table& tbl, RenderSettings& settings) {
  if (const auto name = tbl["render"]["tonemap"].value<std::string>()) {
    if      (*name == "none")     settings.tonemap = Tonemap::None;
    else if (*name == "reinhard") settings.tonemap = Tonemap::Reinhard;
    else { log_error("Unknown tonemap '{}'", *name); return false; }
  }

  return true;
}
}

std::optional<RenderSettings> load_settings(const std::string& path) {
  toml::table tbl;
  try {
    tbl = toml::parse_file(path);
  } catch (const toml::parse_error& err) {
    log_error("Could not read config ({}): {}", path, err.description());
    return std::nullopt;
  }

  RenderSettings settings;
  settings.width     = tbl["render"]["width"].value_or(settings.width);
  settings.height    = tbl["render"]["height"].value_or(settings.height);
  settings.spp       = tbl["render"]["spp"].value_or(settings.spp);
  settings.max_depth = tbl["render"]["max_depth"].value_or(settings.max_depth);
  bool ok = true;
  ok &= read_render_tonemap(tbl, settings);
  
  settings.scene_path  = tbl["scene"]["file"].value_or(settings.scene_path);
  settings.output_path = tbl["output"]["file"].value_or(settings.output_path);

  
  if (settings.width <= 0 || settings.height <= 0) {
    log_error("Invalid resolution: {}x{}", settings.width, settings.height);
    ok = false;
  }
  if (settings.spp <= 0) {
    log_error("Invalid spp (must be > 0) (given: {})", settings.spp);
    ok = false;
  }
  if (settings.max_depth <= 0) {
    log_error("Invalid max_depth (must be > 0) (given: {})", settings.max_depth);
    ok = false;
  }
  if (settings.output_path.empty()) {
    log_error("Output path cannot be empty");
    ok = false;
  }

  if (!ok) {
    return std::nullopt;
  }
  
  return settings;
}

}