#pragma once

// Internal headers.
#include "events.hpp"

// Standard libraries.
#include <functional>
#include <string>

namespace bonicpp::watchdog {

class BaseObserver {
public:
  using Callback = std::function<void(const std::string&, EventType)>;

  BaseObserver() = default;
  virtual ~BaseObserver();

  virtual auto run() -> void = 0;
  auto start() -> void;
  void stop();

protected:
  bool running_ = false;
};

#ifdef __linux__
class InotifyObserver : public BaseObserver {
public:
  using BaseObserver::BaseObserver;
  ~InotifyObserver() override;

  auto run() -> void override;
  auto schedule(const std::string& path, Callback cb) -> void;

private:
  int fd_ = -1;
  int wd_ = -1;
  std::string path_;
  Callback callback_;
};
using Observer = InotifyObserver;
#endif

#ifdef _WIN32
class WindowsApiObserver : public BaseObserver {
public:
  using BaseObserver::BaseObserver;

  auto schedule(const std::string& path, Callback cb) -> void;
  auto run() -> void override;

private:
  HANDLE handle_ = INVALID_HANDLE_VALUE;
  OVERLAPPED overlapped_;
  std::string path_;
  Callback callback_;
};
using Observer = WindowsApiObserver;
#endif

} // namespace bonicpp::watchdog
