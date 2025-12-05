#pragma once

// Standard libraries.
#include <cstdint>
#include <string>
#include <variant>

namespace bonicpp::watchdog {

enum class EventType : std::uint8_t {
  Closed,
  Created,
  Deleted,
  Modified,
  Moved,
  MovedFrom,
  MovedTo
};

template <EventType Type, bool IsDirectory> class PathEvent {
  static_assert(Type != EventType::Closed or not IsDirectory);

public:
  PathEvent(std::string src_path) : src_path_(std::move(src_path)) {}
  auto src_path() -> std::string { return src_path_; }
  static constexpr EventType type = Type;
  static constexpr bool is_directory = IsDirectory;

private:
  std::string src_path_;
};

template <bool IsDirectory>
class PathEvent<EventType::Moved, IsDirectory> {
public:
  PathEvent(std::string src_path, std::string dest_path)
      : src_path_(std::move(src_path)),
        dest_path_(std::move(dest_path)) {}
  auto src_path() -> std::string { return src_path_; }
  auto dest_path() -> std::string { return dest_path_; }
  static constexpr EventType type = EventType::Moved;
  static constexpr bool is_directory = IsDirectory;

private:
  std::string src_path_;
  std::string dest_path_;
};

using DirCreatedEvent = PathEvent<EventType::Created, true>;
using DirDeletedEvent = PathEvent<EventType::Deleted, true>;
using DirModifiedEvent = PathEvent<EventType::Modified, true>;
using DirMovedEvent = PathEvent<EventType::Moved, true>;
using FileClosedEvent = PathEvent<EventType::Closed, false>;
using FileCreatedEvent = PathEvent<EventType::Created, false>;
using FileDeletedEvent = PathEvent<EventType::Deleted, false>;
using FileModifiedEvent = PathEvent<EventType::Modified, false>;
using FileMovedEvent = PathEvent<EventType::Moved, false>;

using Event = std::variant<
    DirCreatedEvent, DirDeletedEvent, DirModifiedEvent, DirMovedEvent,
    FileClosedEvent, FileCreatedEvent, FileDeletedEvent,
    FileModifiedEvent, FileMovedEvent>;

class FileSystemEventHandler {
  auto dispatch(const Event& event) -> void {
    on_any_event(event);
    std::visit(
        [&](auto&& arg) {
          using T = std::decay_t<decltype(arg)>;
          if constexpr (std::is_same_v<T, FileClosedEvent>) {
            on_closed(arg);
          } else if constexpr (
              std::is_same_v<T, DirCreatedEvent> or
              std::is_same_v<T, FileCreatedEvent>) {
            on_created(arg);
          } else if constexpr (
              std::is_same_v<T, DirDeletedEvent> or
              std::is_same_v<T, FileDeletedEvent>) {
            on_deleted(arg);
          } else if constexpr (
              std::is_same_v<T, DirModifiedEvent> or
              std::is_same_v<T, FileModifiedEvent>) {
            on_modified(arg);
          } else if constexpr (
              std::is_same_v<T, DirMovedEvent> or
              std::is_same_v<T, FileMovedEvent>) {
            on_moved(arg);
          }
        },
        event);
  }
  auto on_any_event(const Event& event) -> void { (void)event; }
  auto on_closed(const FileClosedEvent& event) -> void { (void)event; }
  auto on_created(
      const std::variant<DirCreatedEvent, FileCreatedEvent>& event)
      -> void {
    (void)event;
  }
  auto on_deleted(
      const std::variant<DirDeletedEvent, FileDeletedEvent>& event)
      -> void {
    (void)event;
  }
  auto on_modified(
      const std::variant<DirModifiedEvent, FileModifiedEvent>& event)
      -> void {
    (void)event;
  }
  auto on_moved(const std::variant<DirMovedEvent, FileMovedEvent>& event)
      -> void {
    (void)event;
  }
};

auto eventToString(EventType e) -> const char*;

} // namespace bonicpp::watchdog
