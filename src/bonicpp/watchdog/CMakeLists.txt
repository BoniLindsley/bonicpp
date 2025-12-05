find_package(SDL3 CONFIG REQUIRED)
find_package(argparse CONFIG REQUIRED)
find_package(boost_safe_numerics CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

set(TARGET_NAME "${PROJECT_NAME}-watchdog")
add_executable("${TARGET_NAME}" "main.cpp")
set_target_properties(
  "${TARGET_NAME}" PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF
)

target_link_libraries("${TARGET_NAME}" PRIVATE "${PROJECT_NAME}-common")
target_link_libraries("${TARGET_NAME}" PRIVATE argparse::argparse)
target_link_libraries("${TARGET_NAME}" PRIVATE spdlog::spdlog)
