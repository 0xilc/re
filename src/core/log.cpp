#include "log.h"
#include <cstdio>

namespace {
re::LogLevel g_min = re::LogLevel::Info;

const char* prefix_of(re::LogLevel l) {
  switch (l) {
    case re::LogLevel::Info:  return "INFO";
    case re::LogLevel::Warn:  return "WARN";
    case re::LogLevel::Error: return "ERROR";
    default:                  return "?";
  }
}
}

namespace re {
  
void set_log_level(LogLevel level) {g_min = level; }

void detail::vlog(LogLevel level, std::string_view fmt, std::format_args args) { 
  if (level < g_min) return;
  const std::string line = std::format("[{}] {}\n", prefix_of(level), std::vformat(fmt, args));
  std::fwrite(line.data(), 1, line.size(), stderr);
}
}
