// External dependencies.
#include <argparse/argparse.hpp>
#include <spdlog/cfg/env.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

// Standard libraries.
#include <iostream>

auto run() -> int { return 0; }

auto main(int argc, char** argv) -> int {
  auto program_name = "bonicpp";
  auto program_version = "0.0.1";
  argparse::ArgumentParser program(
      program_name, program_version, argparse::default_arguments::none);
  program.add_epilog(
      "Logging level can be changed by setting $SPDLOG_LEVEL"
      " to 'critical', 'error', 'warn', 'info', 'debug' or 'trace'."
      " Override with '--quiet' and '--verbose'.");

  auto help_count = 0;
  program.add_argument("-h", "--help")
      .action([&](const auto&) { ++help_count; })
      .default_value(false)
      .help("Shows help message and exits.")
      .implicit_value(true)
      .nargs(0);

  auto verbose_count = 0;
  program.add_argument("-v", "--verbose")
      .action([&](const auto&) { ++verbose_count; })
      .append()
      .default_value(false)
      .help("Print debugging information. Overrides `$SPDLOG_LEVEL`")
      .implicit_value(true)
      .nargs(0);

  program.add_argument("-q", "--quiet")
      .action([&](const auto&) { --verbose_count; })
      .append()
      .default_value(false)
      .help("Print less debugging information. Stacks with `--verbose`.")
      .implicit_value(true)
      .nargs(0);

  try {
    program.parse_args(argc, argv);
  } catch (const std::exception& error) {
    std::cerr << error.what() << std::endl;
    std::cerr << program;
    return 1;
  }

  if (help_count > 0) {
    std::cerr << program;
    return 0;
  }

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

  return run();
}
