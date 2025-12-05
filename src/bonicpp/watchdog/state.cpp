// Corresponding header.
#include "state.hpp"

// Internal headers.
#include "observers.hpp"

// External dependencies.
#include <spdlog/spdlog.h>

// Standard libraries.
#include <string>

namespace bonicpp::watchdog {

auto State::add_arguments(::argparse::ArgumentParser& program) -> void {
  program.add_argument("directory")
      .action([&](const std::string& value) { directory = value; });
}

auto State::process_arguments(const ::argparse::ArgumentParser& program)
    -> void {
  (void)program;
}

auto State::run() -> int {
  try {
    auto monitor = FileMonitor(
        directory, [](const std::string& file, EventType event) {
          spdlog::info("[{}] {}", eventToString(event), file);
        });

    monitor.start();
  } catch (const std::exception& e) {
    spdlog::error("Error: {}", e.what());
    return 1;
  }
  return 0;
}

} // namespace bonicpp::watchdog
