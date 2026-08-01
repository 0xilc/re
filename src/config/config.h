#pragma once

#include "config/render_settings.h"
#include <string>
#include <optional>

namespace re {

std::optional<RenderSettings> load_settings(const std::string& path);

}