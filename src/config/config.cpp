#include "config/config.h"

#include <string>
#include <toml.hpp>

#include "config/render_settings.h"
#include "core/log.h"

namespace re {

namespace {
const char* to_string(Tonemap t) {
    switch (t) {
        case Tonemap::None:     return "none";
        case Tonemap::Reinhard: return "reinhard";
        case Tonemap::AgX:      return "agx";
    }
    return "?";
}

const char* to_string(IntegratorMode m) {
    switch (m) {
        case IntegratorMode::Naive: return "naive";
        case IntegratorMode::Nee:   return "nee";
        case IntegratorMode::Mis:   return "mis";
    }
    return "?";
}

bool read_tonemap(const toml::table& tbl, RenderSettings& settings) {
    const auto name = tbl["render"]["tonemap"].value<std::string>();
    if (!name) return true;

    if      (*name == "none")     settings.tonemap = Tonemap::None;
    else if (*name == "reinhard") settings.tonemap = Tonemap::Reinhard;
    else if (*name == "agx")      settings.tonemap = Tonemap::AgX;
    else {
        log_error("Unknown tonemap '{}' (expected none, reinhard)", *name);
        return false;
    }
    return true;
}

bool read_integrator(const toml::table& tbl, PathTracerSettings& pt) {
    const auto name = tbl["path_tracer"]["integrator"].value<std::string>();
    if (!name) return true;

    if      (*name == "naive") pt.integrator = IntegratorMode::Naive;
    else if (*name == "nee")   pt.integrator = IntegratorMode::Nee;
    else if (*name == "mis")   pt.integrator = IntegratorMode::Mis;
    else {
        log_error("Unknown integrator '{}' (expected naive, nee or mis)", *name);
        return false;
    }
    return true;
}

bool read_render(const toml::table& tbl, RenderSettings& settings) {
    settings.width   = tbl["render"]["width"].value_or(settings.width);
    settings.height  = tbl["render"]["height"].value_or(settings.height);
    settings.threads = tbl["render"]["threads"].value_or(settings.threads);

    bool ok = read_tonemap(tbl, settings);

    if (settings.width <= 0 || settings.height <= 0) {
        log_error("Invalid resolution: {}x{}", settings.width, settings.height);
        ok = false;
    }

    if (settings.threads < 0) {
        log_error("Invalid threads (must be >= 0, 0 means all cores) (given: {})",
                  settings.threads);
        ok = false;
    }
    return ok;
}

bool read_path_tracer(const toml::table& tbl, PathTracerSettings& pt) {
    pt.spp       = tbl["path_tracer"]["spp"].value_or(pt.spp);
    pt.max_depth = tbl["path_tracer"]["max_depth"].value_or(pt.max_depth);

    bool ok = read_integrator(tbl, pt);

    if (pt.spp <= 0) {
        log_error("Invalid spp (must be > 0) (given: {})", pt.spp);
        ok = false;
    }
    if (pt.max_depth <= 0) {
        log_error("Invalid max_depth (must be > 0) (given: {})", pt.max_depth);
        ok = false;
    }
    return ok;
}

bool read_paths(const toml::table& tbl, RenderSettings& settings) {
    settings.scene_path  = tbl["scene"]["file"].value_or(settings.scene_path);
    settings.output_path = tbl["output"]["file"].value_or(settings.output_path);

    if (settings.output_path.empty()) {
        log_error("Output path cannot be empty");
        return false;
    }
    return true;
}

}  // namespace

void log_settings(const RenderSettings& settings) {
    log_info("render: {}x{} threads={} tonemap={}", settings.width, settings.height, settings.threads,
             to_string(settings.tonemap));
    log_info("path tracer: spp={} max_depth={} integrator={}", settings.path_tracer.spp,
             settings.path_tracer.max_depth, to_string(settings.path_tracer.integrator));
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

    bool ok = true;
    ok = read_render(tbl, settings) && ok;
    ok = read_path_tracer(tbl, settings.path_tracer) && ok;
    ok = read_paths(tbl, settings) && ok;

    if (!ok) return std::nullopt;
    return settings;
}

}  // namespace re