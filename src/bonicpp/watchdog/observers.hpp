#pragma once

// Internal headers.
#include "events.hpp"

// Standard libraries.
#include <functional>
#include <string>

namespace bonicpp::watchdog {

class Observer {
public:
  using Callback = std::function<void(const std::string&, EventType)>;

  Observer(const std::string& path, Callback cb);
  ~Observer();

  auto start() -> void;
  void stop();

  auto monitor() -> void;

private:
#ifdef __linux__
  int fd_ = -1;
  int wd_ = -1;
#elif _WIN32
  HANDLE handle_ = INVALID_HANDLE_VALUE;
  OVERLAPPED overlapped_;
#endif

  std::string path_;
  Callback callback_;
  bool running_ = false;
};

} // namespace bonicpp::watchdog
