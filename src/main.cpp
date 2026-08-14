#include "config/config.h"
#include "config/render_settings.h"
#include "core/log.h"
#include "driver/offline_driver.h"

int main(int argc, char** argv) {
    re::log_info("re, re, raaaw!");

    const char* config_path = argc > 1 ? argv[1] : "configs/default.toml";

    const std::optional<re::RenderSettings> loaded = re::load_settings(config_path);
    if (!loaded) return 1;

    const re::RenderSettings settings = *loaded;
    re::log_settings(settings);

    return re::run_offline(settings) ? 0 : 1;
}