#pragma once

#include <format>
#include <string_view>

namespace re {

enum class LogLevel {
  Info,
  Warn,
  Error,
  Off
};

void set_log_level(LogLevel level);

namespace detail {
  void vlog(LogLevel level, std::string_view fmt, std::format_args);
}

template <typename... Args>
void log_info(std::format_string<Args...> fmt, Args&&... args) {
  detail::vlog(LogLevel::Info, fmt.get(), std::make_format_args(args...));
}

template <typename... Args>
void log_warn(std::format_string<Args...> fmt, Args&&... args) {
  detail::vlog(LogLevel::Warn, fmt.get(), std::make_format_args(args...));
}

template <typename... Args>
void log_error(std::format_string<Args...> fmt, Args&&... args) {
  detail::vlog(LogLevel::Error, fmt.get(), std::make_format_args(args...));
}
}