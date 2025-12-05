// Corresponding header.
#include "logging.hpp"

// External dependencies.
#include <argparse/argparse.hpp>
#include <spdlog/cfg/env.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace bonicpp::logging {

auto State::add_arguments(argparse::ArgumentParser& program) -> void {
  program.add_argument("-v", "--verbose")
      .action([&](const auto&) { ++verbose_count; })
      .append()
      .default_value(false)
      .help(
          "Print debugging information."
          " Overrides `$SPDLOG_LEVEL` which can be"
          " 'critical', 'error', 'warn', 'info', 'debug' or 'trace'.")
      .implicit_value(true)
      .nargs(0);

  program.add_argument("-q", "--quiet")
      .action([&](const auto&) { --verbose_count; })
      .append()
      .default_value(false)
      .help("Print less debugging information. Stacks with `--verbose`.")
      .implicit_value(true)
      .nargs(0);
}

auto State::process_arguments(const argparse::ArgumentParser& program)
    -> void {
  // Log to stderr.
  auto logger = spdlog::stderr_color_mt("default");
  spdlog::set_default_logger(logger);

  // Set up log level.
  if (program.is_used("--verbose") or program.is_used("--quiet")) {
    // There are 7 levels in total, enumerated in "quietness" order.
    // -   0: trace
    // -   1: debug
    // -   2: info
    // -   3: warn
    // -   4: eror
    // -   5: critical
    // -   6: off
    // Convert from verbosity flags.
    // -   -4: off
    // -   -3: critical
    // -   -2: error
    // -   -1: warn
    // -   0: info, currently default spdlog log level.
    // -   1: debug
    // -   2: trace
    auto default_log_level = spdlog::get_level();
    auto log_level =
        spdlog::level::level_enum{default_log_level - verbose_count};
    if (log_level < spdlog::level::trace) {
      log_level = spdlog::level::trace;
    } else if (log_level >= spdlog::level::off) {
      log_level = spdlog::level::off;
    }
    spdlog::set_level(log_level);
    spdlog::trace(
        "Command line sets verbosity to {}.",
        static_cast<int>(log_level));
  } else {
    // Use environment variable for logging level by default.
    spdlog::cfg::load_env_levels();
  }
  {
    auto log_level = spdlog::get_level();
    spdlog::debug(
        "Logging initialised to level {} ({}).",
        static_cast<int>(log_level),
        spdlog::level::to_string_view(log_level));
  }
}

} // namespace bonicpp::logging
