find_package(argparse CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

set(TARGET_NAME "${PROJECT_NAME}-watchdog-common")
add_library("${TARGET_NAME}" "events.cpp" "observers.cpp" "state.cpp")
set_target_properties(
  "${TARGET_NAME}" PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF
)
target_link_libraries("${TARGET_NAME}" PRIVATE "${PROJECT_NAME}-common")
target_link_libraries("${TARGET_NAME}" PRIVATE argparse::argparse)
target_link_libraries("${TARGET_NAME}" PRIVATE spdlog::spdlog)

set(TARGET_NAME "${PROJECT_NAME}-watchdog")
add_executable("${TARGET_NAME}" "main.cpp")
set_target_properties(
  "${TARGET_NAME}" PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF
)

target_link_libraries(
  "${TARGET_NAME}" PRIVATE "${PROJECT_NAME}-watchdog-common"
)
target_link_libraries("${TARGET_NAME}" PRIVATE "${PROJECT_NAME}-common")
target_link_libraries("${TARGET_NAME}" PRIVATE argparse::argparse)
target_link_libraries("${TARGET_NAME}" PRIVATE spdlog::spdlog)
