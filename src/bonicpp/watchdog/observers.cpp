// Corresponding header.
#include "observers.hpp"

// Internal headers.
#include "events.hpp"

// External dependencies.
#include <spdlog/spdlog.h>

// Standard libraries.
#include <array>
#include <stdexcept>
#include <string>

// System libraries.
#ifdef __linux__
#include <poll.h>
#include <sys/inotify.h>
#include <unistd.h>
#elif _WIN32
#include <windows.h>
#endif

namespace bonicpp::watchdog {

FileMonitor::FileMonitor(const std::string& path, Callback cb)
    : path_(path), callback_(std::move(cb)) {
#ifdef __linux__
  fd_ = inotify_init1(IN_NONBLOCK);
  if (fd_ == -1) {
    throw std::runtime_error("Failed to initialize inotify");
  }

  wd_ = inotify_add_watch(
      fd_, path.c_str(),
      IN_CREATE | IN_MODIFY | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO);
  if (wd_ == -1) {
    close(fd_);
    throw std::runtime_error("Failed to add watch: " + path);
  }
#elif _WIN32
  handle_ = CreateFileA(
      path.c_str(), FILE_LIST_DIRECTORY,
      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
      OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
      NULL);
  if (handle_ == INVALID_HANDLE_VALUE) {
    throw std::runtime_error("Failed to open directory: " + path);
  }
  memset(&overlapped_, 0, sizeof(overlapped_));
  overlapped_.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
#endif
}
FileMonitor::~FileMonitor() {
  stop();
#ifdef __linux__
  if (wd_ != -1)
    inotify_rm_watch(fd_, wd_);
  if (fd_ != -1)
    close(fd_);
#elif _WIN32
  if (overlapped_.hEvent)
    CloseHandle(overlapped_.hEvent);
  if (handle_ != INVALID_HANDLE_VALUE)
    CloseHandle(handle_);
#endif
}

auto FileMonitor::start() -> void {
  running_ = true;
  spdlog::info("Monitoring: {}", path_);
  monitor();
}

auto FileMonitor::stop() -> void { running_ = false; }

#ifdef __linux__
auto FileMonitor::monitor() -> void {
  std::array<char, 4096> buf;
  struct pollfd pfd = {.fd = fd_, .events = POLLIN, .revents = 0};

  while (running_) {
    int poll_ret = poll(&pfd, 1, 1000);
    if (poll_ret == -1) {
      if (errno == EINTR)
        continue;
      throw std::runtime_error("Poll failed");
    }

    if (poll_ret == 0)
      continue;

    ssize_t len = read(fd_, buf.data(), buf.size());
    if (len == -1 && errno != EAGAIN) {
      throw std::runtime_error("Read failed");
    }

    for (char* ptr = buf.data(); ptr < buf.data() + len;) {
      auto event = reinterpret_cast<inotify_event*>(ptr);

      if (event->len) {
        EventType fe;
        if (event->mask & IN_CREATE)
          fe = EventType::Created;
        else if (event->mask & IN_MODIFY)
          fe = EventType::Modified;
        else if (event->mask & IN_DELETE)
          fe = EventType::Deleted;
        else if (event->mask & IN_MOVED_FROM)
          fe = EventType::MovedFrom;
        else if (event->mask & IN_MOVED_TO)
          fe = EventType::MovedTo;
        else {
          ptr += sizeof(struct inotify_event) + event->len;
          continue;
        }

        callback_(event->name, fe);
      }

      ptr += sizeof(struct inotify_event) + event->len;
    }
  }
}
#elif _WIN32
auto FileMonitor::monitor() -> void {
  char buffer[4096];
  DWORD bytesReturned;

  while (running_) {
    BOOL success = ReadDirectoryChangesW(
        handle_, buffer, sizeof(buffer), FALSE,
        FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
        &bytesReturned, &overlapped_, NULL);

    if (!success) {
      throw std::runtime_error("ReadDirectoryChangesW failed");
    }

    DWORD result = WaitForSingleObject(overlapped_.hEvent, 1000);
    if (result == WAIT_TIMEOUT)
      continue;
    if (result != WAIT_OBJECT_0)
      break;

    GetOverlappedResult(handle_, &overlapped_, &bytesReturned, FALSE);
    ResetEvent(overlapped_.hEvent);

    FILE_NOTIFY_INFORMATION* info = (FILE_NOTIFY_INFORMATION*)buffer;
    while (true) {
      std::wstring wname(
          info->FileName, info->FileNameLength / sizeof(WCHAR));
      std::string name(wname.begin(), wname.end());

      EventType fe;
      switch (info->Action) {
      case FILE_ACTION_ADDED:
        fe = EventType::Created;
        break;
      case FILE_ACTION_REMOVED:
        fe = EventType::Deleted;
        break;
      case FILE_ACTION_MODIFIED:
        fe = EventType::Modified;
        break;
      case FILE_ACTION_RENAMED_OLD_NAME:
        fe = EventType::MovedFrom;
        break;
      case FILE_ACTION_RENAMED_NEW_NAME:
        fe = EventType::MovedTo;
        break;
      default:
        goto next;
      }

      callback_(name, fe);

    next:
      if (info->NextEntryOffset == 0)
        break;
      info = (FILE_NOTIFY_INFORMATION*)((char*)info +
                                        info->NextEntryOffset);
    }
  }
}
#endif
} // namespace bonicpp::watchdog
