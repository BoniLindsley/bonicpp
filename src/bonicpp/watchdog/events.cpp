// Corresponding header.
#include "events.hpp"

namespace bonicpp::watchdog {

auto eventToString(EventType e) -> const char* {
  switch (e) {
  case EventType::Created:
    return "Created";
  case EventType::Modified:
    return "Modified";
  case EventType::Deleted:
    return "Deleted";
  case EventType::MovedFrom:
    return "MovedFrom";
  case EventType::MovedTo:
    return "MovedTo";
  default:
    return "Unknown";
  }
}

} // namespace bonicpp::watchdog
